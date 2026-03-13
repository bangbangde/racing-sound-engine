#include "cylinder_bank.h"
#include <cmath>
#include <cstring>
#include <algorithm>

namespace engin {

CylinderBank::CylinderBank() {
    reset();
}

void CylinderBank::configure(const EnginePreset& preset) {
    num_cylinders_ = preset.num_cylinders;
    for (int i = 0; i < MAX_CYLINDERS; ++i) {
        phase_offsets_[i] = preset.phase_offsets[i];
    }
    impulse_duration_  = preset.impulse_duration;
    impulse_decay_     = preset.impulse_decay;
    attack_sharpness_  = preset.combustion_attack_sharpness;
    ring_frequency_    = preset.combustion_ring_freq;
    ring_amount_       = preset.combustion_ring_amount;
    jitter_amount_     = preset.combustion_jitter;
    timing_jitter_     = preset.timing_jitter;
    reset();
}

void CylinderBank::reset() {
    crank_phase_ = 0.0f;
    for (int i = 0; i < MAX_CYLINDERS; ++i) {
        cylinders_[i].phase = phase_offsets_[i];
        cylinders_[i].last_phase = phase_offsets_[i];
        cylinders_[i].impulse_time = 1.0f;
        cylinders_[i].amplitude_jitter = 1.0f;
    }
    rng_state_[0] = 123456789;
    rng_state_[1] = 362436069;
    rng_state_[2] = 521288629;
    rng_state_[3] = 88675123;
}

float CylinderBank::xorshift128() {
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

void CylinderBank::process(Sample* output, FrameCount frames, float firing_freq,
                            float throttle, SampleRate sample_rate) {
    if (firing_freq <= 0.0f || num_cylinders_ <= 0) return;

    // One full crank cycle = TWO_PI. Each cylinder fires once per cycle.
    // phase_increment per sample for the crank
    const float crank_freq = firing_freq / static_cast<float>(num_cylinders_);
    const float phase_inc = TWO_PI * crank_freq / static_cast<float>(sample_rate);

    // Impulse parameters modulated by throttle
    const float duration = impulse_duration_ * (1.0f - 0.5f * throttle); // shorter at high throttle
    const float decay = impulse_decay_ * (1.0f + 2.0f * throttle);      // faster decay at high throttle
    const float amplitude = 0.3f + 0.7f * throttle;
    const float dt = 1.0f / static_cast<float>(sample_rate);
    const float attack_time = duration * 0.15f; // attack phase is ~15% of total duration

    for (FrameCount f = 0; f < frames; ++f) {
        float sample_val = 0.0f;

        for (int c = 0; c < num_cylinders_; ++c) {
            auto& cyl = cylinders_[c];

            // Current cylinder phase = crank_phase + offset, wrapped to [0, TWO_PI)
            float cyl_phase = crank_phase_ + phase_offsets_[c];
            // Normalize
            cyl_phase = std::fmod(cyl_phase, TWO_PI);
            if (cyl_phase < 0.0f) cyl_phase += TWO_PI;

            float prev_phase = cyl.last_phase;
            // Detect firing: phase crosses zero (wraps from ~TWO_PI to ~0)
            if (prev_phase > PI && cyl_phase <= PI && prev_phase - cyl_phase > PI) {
                // Apply combustion jitter on each firing event
                cyl.amplitude_jitter = 1.0f + jitter_amount_ * xorshift128();
                float time_offset = timing_jitter_ * xorshift128();
                cyl.impulse_time = std::max(0.0f, time_offset);
            }
            cyl.last_phase = cyl_phase;

            // Generate impulse if active -- asymmetric waveform
            if (cyl.impulse_time < duration) {
                float t = cyl.impulse_time;
                float env;

                if (t < attack_time) {
                    // Fast attack phase: rapid pressure rise
                    env = 1.0f - std::exp(-attack_sharpness_ * t);
                } else {
                    // Ring-down decay phase with oscillation
                    float t_decay = t - attack_time;
                    float ring = 1.0f + ring_amount_ * std::sin(TWO_PI * ring_frequency_ * t_decay);
                    env = std::exp(-decay * t_decay) * ring;
                }

                sample_val += amplitude * cyl.amplitude_jitter * env;
            }

            cyl.impulse_time += dt;
        }

        output[f] += sample_val;
        crank_phase_ += phase_inc;
        crank_phase_ = std::fmod(crank_phase_, TWO_PI);
    }
}

} // namespace engin
