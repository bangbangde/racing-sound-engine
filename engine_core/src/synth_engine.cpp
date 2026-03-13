#include "synth_engine.h"
#include <cmath>
#include <cstring>
#include <algorithm>

namespace engin {

SynthEngine::SynthEngine() = default;
SynthEngine::~SynthEngine() = default;

void SynthEngine::init(SampleRate sample_rate) {
    sample_rate_ = sample_rate;
    sample_player_.set_sample_rate(sample_rate);
}

void SynthEngine::load_preset(const EnginePreset& preset) {
    // Store a copy and publish pointer
    cached_preset_ = preset;
    current_preset_.store(&cached_preset_, std::memory_order_release);

    // Configure sub-modules
    cylinder_bank_.configure(preset);
    harmonic_synth_.set_harmonic_profile(preset.harmonic_amplitudes, preset.num_harmonics);

    // Exhaust resonant filter bank (Peak filters in series)
    num_exhaust_resonances_ = std::min(preset.num_exhaust_resonances, MAX_EXHAUST_RESONANCES);
    for (int i = 0; i < num_exhaust_resonances_; ++i) {
        exhaust_resonances_[i].set_params(FilterType::Peak,
                                           preset.exhaust_resonance_freqs[i],
                                           preset.exhaust_resonance_Qs[i],
                                           preset.exhaust_resonance_gains_db[i],
                                           sample_rate_);
    }
    // High-frequency roll-off: simulates exhaust pipe acoustic filtering
    exhaust_lowpass_.set_params(FilterType::LowPass, preset.exhaust_lowpass_freq,
                                preset.exhaust_lowpass_Q, 0.0f, sample_rate_);

    intake_filter_.set_params(FilterType::BandPass, preset.intake_resonance_freq,
                               preset.intake_resonance_Q, 0.0f, sample_rate_);
    distortion_.set_params(preset.distortion_drive, preset.distortion_mix);
    reverb_.set_params(preset.reverb_room_size, preset.reverb_damping, preset.reverb_mix);

    intake_noise_.set_params(preset.intake_noise_center_freq,
                              preset.intake_noise_bandwidth,
                              preset.intake_noise_level);
    exhaust_noise_.set_params(preset.exhaust_noise_center_freq,
                               preset.exhaust_noise_bandwidth,
                               preset.exhaust_noise_level);

    turbo_whistle_.set_params(preset.turbo_base_freq, preset.turbo_freq_range,
                               preset.turbo_noise_mix, preset.turbo_max_amplitude);
}

void SynthEngine::set_params(const EngineParams& params) {
    params_.write(params);
}

int SynthEngine::load_sample(const float* pcm_data, uint32_t num_frames,
                              SampleRate original_rate, bool loop) {
    return sample_player_.load_sample(pcm_data, num_frames, original_rate, loop);
}

void SynthEngine::reset() {
    cylinder_bank_.reset();
    harmonic_synth_.reset();
    intake_noise_.reset();
    exhaust_noise_.reset();
    sample_player_.stop_all();
    turbo_whistle_.reset();
    for (int i = 0; i < MAX_EXHAUST_RESONANCES; ++i) {
        exhaust_resonances_[i].reset();
    }
    exhaust_lowpass_.reset();
    intake_filter_.reset();
    distortion_.reset();
    reverb_.reset();
    mixer_.clear_all();
}

void SynthEngine::process(Sample* output, FrameCount frames) {
    if (frames > MAX_SCRATCH) frames = MAX_SCRATCH;

    // Read parameters
    const EngineParams& p = params_.read();
    const EnginePreset* preset = current_preset_.load(std::memory_order_acquire);
    if (!preset) {
        std::memset(output, 0, frames * sizeof(Sample));
        return;
    }

    // Compute firing frequency: (RPM * cylinders) / (60 * strokes/2)
    float firing_freq = (p.rpm * static_cast<float>(preset->num_cylinders))
                        / (60.0f * static_cast<float>(preset->num_strokes) / 2.0f);
    // For harmonic synth: fundamental = single cylinder firing rate
    float fundamental_freq = p.rpm / 60.0f; // rotations per second
    float load = p.throttle; // simplified: load ~ throttle

    // Normalized RPM for brightness modulation (0 at idle, 1 at redline)
    float rpm_range = preset->redline_rpm - preset->idle_rpm;
    float rpm_norm = (rpm_range > 0.0f)
        ? std::clamp((p.rpm - preset->idle_rpm) / rpm_range, 0.0f, 1.0f)
        : 0.0f;

    // 1. Clear scratch buffers
    std::memset(scratch_cylinders_, 0, frames * sizeof(Sample));
    std::memset(scratch_harmonics_, 0, frames * sizeof(Sample));
    std::memset(scratch_intake_noise_, 0, frames * sizeof(Sample));
    std::memset(scratch_exhaust_noise_, 0, frames * sizeof(Sample));
    std::memset(scratch_samples_, 0, frames * sizeof(Sample));

    // 2. Cylinder impulse layer -> exhaust resonant filters -> lowpass roll-off -> distortion -> channel 0
    cylinder_bank_.process(scratch_cylinders_, frames, firing_freq, p.throttle, sample_rate_);
    for (int i = 0; i < num_exhaust_resonances_; ++i) {
        exhaust_resonances_[i].process(scratch_cylinders_, frames);
    }
    exhaust_lowpass_.process(scratch_cylinders_, frames);
    distortion_.process(scratch_cylinders_, frames);
    mixer_.mix_into(0, scratch_cylinders_, frames);

    // 3. Harmonic synthesis layer -> intake filter -> channel 1
    harmonic_synth_.process(scratch_harmonics_, frames, fundamental_freq, load, rpm_norm, sample_rate_);
    intake_filter_.process(scratch_harmonics_, frames);
    mixer_.mix_into(1, scratch_harmonics_, frames);

    // 4. Noise layers -> channels 2,3
    intake_noise_.process(scratch_intake_noise_, frames, p.throttle * 0.5f + 0.2f, sample_rate_);
    mixer_.mix_into(2, scratch_intake_noise_, frames);

    exhaust_noise_.process(scratch_exhaust_noise_, frames, p.throttle * 0.7f + 0.1f, sample_rate_);
    mixer_.mix_into(3, scratch_exhaust_noise_, frames);

    // 5. Sample player -> channel 4
    sample_player_.process(scratch_samples_, frames);
    mixer_.mix_into(4, scratch_samples_, frames);

    // 6. Turbo whistle -> channel 5 (only when enabled)
    if (p.turbo_enabled) {
        std::memset(scratch_turbo_, 0, frames * sizeof(Sample));
        turbo_whistle_.process(scratch_turbo_, frames, rpm_norm, p.throttle, sample_rate_);
        mixer_.mix_into(5, scratch_turbo_, frames);
    }

    // 7. Mix all channels
    mixer_.render(output, frames);

    // 8. Master effects: reverb
    reverb_.process(output, frames);

    // 9. Soft limiter: tanh compression to prevent clipping
    float master_gain = 0.8f;
    for (FrameCount i = 0; i < frames; ++i) {
        output[i] = std::tanh(output[i]) * master_gain;
    }
}

} // namespace engin
