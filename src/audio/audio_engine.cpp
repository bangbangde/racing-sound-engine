#include "audio/audio_engine.h"
#include "miniaudio.h"
#include <cmath>
#include <cstring>

namespace engin {

struct AudioEngine::MaDeviceWrapper {
    ma_device device;
};

AudioEngine::AudioEngine() = default;

AudioEngine::~AudioEngine() {
    shutdown();
}

bool AudioEngine::init(const AudioConfig& config) {
    sample_rate_   = config.sample_rate;
    buffer_frames_ = config.buffer_frames;

    device_ = new MaDeviceWrapper();

    ma_device_config dev_config = ma_device_config_init(ma_device_type_playback);
    dev_config.playback.format   = ma_format_f32;
    dev_config.playback.channels = 1; // mono
    dev_config.sampleRate        = sample_rate_;
    dev_config.periodSizeInFrames = buffer_frames_;
    dev_config.dataCallback      = [](ma_device* dev, void* out, const void* /*in*/, ma_uint32 fc) {
        auto* engine = static_cast<AudioEngine*>(dev->pUserData);
        engine->process_audio(static_cast<Sample*>(out), static_cast<FrameCount>(fc));
    };
    dev_config.pUserData = this;

    if (ma_device_init(nullptr, &dev_config, &device_->device) != MA_SUCCESS) {
        delete device_;
        device_ = nullptr;
        return false;
    }

    // Configure sample player sample rate
    sample_player_.set_sample_rate(sample_rate_);

    return true;
}

void AudioEngine::start() {
    if (device_ && !running_.load()) {
        ma_device_start(&device_->device);
        running_.store(true, std::memory_order_release);
    }
}

void AudioEngine::stop() {
    if (device_ && running_.load()) {
        running_.store(false, std::memory_order_release);
        ma_device_stop(&device_->device);
    }
}

void AudioEngine::shutdown() {
    stop();
    if (device_) {
        ma_device_uninit(&device_->device);
        delete device_;
        device_ = nullptr;
    }
}

void AudioEngine::load_preset(const EnginePreset& preset) {
    // Store a copy and publish pointer
    cached_preset_ = preset;
    current_preset_.store(&cached_preset_, std::memory_order_release);

    // Configure sub-modules
    cylinder_bank_.configure(preset);
    harmonic_synth_.set_harmonic_profile(preset.harmonic_amplitudes, preset.num_harmonics);

    exhaust_filter_.set_params(FilterType::LowPass, preset.exhaust_resonance_freq,
                                preset.exhaust_resonance_Q, 0.0f, sample_rate_);
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
}

void AudioEngine::set_params(const EngineParams& params) {
    params_.write(params);
}

void AudioEngine::render_to_buffer(Sample* output, FrameCount frames) {
    process_audio(output, frames);
}

void AudioEngine::process_audio(Sample* output, FrameCount frames) {
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
    float load = p.throttle; // simplified: load ≈ throttle

    // 1. Clear scratch buffers
    std::memset(scratch_cylinders_, 0, frames * sizeof(Sample));
    std::memset(scratch_harmonics_, 0, frames * sizeof(Sample));
    std::memset(scratch_intake_noise_, 0, frames * sizeof(Sample));
    std::memset(scratch_exhaust_noise_, 0, frames * sizeof(Sample));
    std::memset(scratch_samples_, 0, frames * sizeof(Sample));

    // 2. Cylinder impulse layer → exhaust filter → distortion → channel 0
    cylinder_bank_.process(scratch_cylinders_, frames, firing_freq, p.throttle, sample_rate_);
    exhaust_filter_.process(scratch_cylinders_, frames);
    distortion_.process(scratch_cylinders_, frames);
    mixer_.mix_into(0, scratch_cylinders_, frames);

    // 3. Harmonic synthesis layer → intake filter → channel 1
    harmonic_synth_.process(scratch_harmonics_, frames, fundamental_freq, load, sample_rate_);
    intake_filter_.process(scratch_harmonics_, frames);
    mixer_.mix_into(1, scratch_harmonics_, frames);

    // 4. Noise layers → channels 2,3
    intake_noise_.process(scratch_intake_noise_, frames, p.throttle * 0.5f + 0.2f, sample_rate_);
    mixer_.mix_into(2, scratch_intake_noise_, frames);

    exhaust_noise_.process(scratch_exhaust_noise_, frames, p.throttle * 0.7f + 0.1f, sample_rate_);
    mixer_.mix_into(3, scratch_exhaust_noise_, frames);

    // 5. Sample player → channel 4
    sample_player_.process(scratch_samples_, frames);
    mixer_.mix_into(4, scratch_samples_, frames);

    // 6. Mix all channels
    mixer_.render(output, frames);

    // 7. Master effects: reverb
    reverb_.process(output, frames);

    // 8. Soft limiter: tanh compression to prevent clipping
    float master_gain = 0.8f;
    for (FrameCount i = 0; i < frames; ++i) {
        output[i] = std::tanh(output[i]) * master_gain;
    }
}

} // namespace engin
