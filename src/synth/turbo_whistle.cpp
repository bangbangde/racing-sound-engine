#include "synth/turbo_whistle.h"
#include <cmath>
#include <algorithm>

namespace engin {

TurboWhistle::TurboWhistle() {
    reset();
}

void TurboWhistle::set_params(float base_freq, float freq_range, float noise_mix, float max_amplitude) {
    base_freq_     = std::max(100.0f, base_freq);
    freq_range_    = std::max(0.0f, freq_range);
    noise_mix_     = std::clamp(noise_mix, 0.0f, 1.0f);
    max_amplitude_ = std::max(0.0f, max_amplitude);
}

void TurboWhistle::reset() {
    phase_ = 0.0f;
    spool_ = 0.0f;
    bp_y1_ = bp_y2_ = 0.0f;
    rng_state_[0] = 98765432;
    rng_state_[1] = 456789012;
    rng_state_[2] = 321654987;
    rng_state_[3] = 112233445;
}

float TurboWhistle::xorshift128() {
    uint32_t t = rng_state_[3];
    t ^= t << 11;
    t ^= t >> 8;
    rng_state_[3] = rng_state_[2];
    rng_state_[2] = rng_state_[1];
    rng_state_[1] = rng_state_[0];
    t ^= rng_state_[0];
    t ^= rng_state_[0] >> 19;
    rng_state_[0] = t;
    return static_cast<float>(static_cast<int32_t>(t)) / 2147483648.0f;
}

void TurboWhistle::process(Sample* output, FrameCount frames, float rpm_normalized,
                            float throttle, SampleRate sample_rate) {
    if (max_amplitude_ <= 0.0f) return;

    const float sr = static_cast<float>(sample_rate);
    const float dt = 1.0f / sr;

    // Spool-up target: turbo boost depends on both RPM and throttle
    float spool_target = rpm_normalized * throttle;

    // Spool smoothing coefficient: turbo has significant lag
    // Spool-up is slower than spool-down (turbo inertia)
    const float spool_up_rate   = 2.0f;  // ~0.5s to spool up
    const float spool_down_rate = 4.0f;  // ~0.25s to spool down

    for (FrameCount f = 0; f < frames; ++f) {
        // One-pole spool smoothing (asymmetric rates)
        float rate = (spool_target > spool_) ? spool_up_rate : spool_down_rate;
        spool_ += (spool_target - spool_) * rate * dt;
        spool_ = std::clamp(spool_, 0.0f, 1.0f);

        // Amplitude ramps up non-linearly with spool (turbo effect is weak at low spool)
        float amplitude = max_amplitude_ * spool_ * spool_;

        if (amplitude < 0.0001f) {
            continue; // skip computation when effectively silent
        }

        // Turbo frequency: rises with spool level
        float turbo_freq = base_freq_ + freq_range_ * spool_;

        // FM jitter: slight frequency instability for realism
        float fm_jitter = 1.0f + 0.02f * xorshift128();
        turbo_freq *= fm_jitter;

        // Tone component: sine oscillator
        float tone = std::sin(phase_);
        phase_ += TWO_PI * turbo_freq / sr;
        phase_ = std::fmod(phase_, TWO_PI);

        // Noise component: filtered white noise for turbulence
        float noise_raw = xorshift128();
        // Simple one-pole bandpass around turbo frequency
        // Using a resonant IIR approximation
        float bp_coeff = std::exp(-TWO_PI * 800.0f / sr); // bandwidth ~800 Hz
        float noise_filtered = noise_raw - bp_coeff * bp_y1_;
        bp_y1_ = noise_raw;

        // Mix tone and noise
        float signal = (1.0f - noise_mix_) * tone + noise_mix_ * noise_filtered;

        output[f] += amplitude * signal;
    }
}

} // namespace engin
