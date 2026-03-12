#include "core/cylinder_bank.h"
#include <cmath>
#include <cstring>

namespace engin {

CylinderBank::CylinderBank() {
    reset();
}

void CylinderBank::configure(const EnginePreset& preset) {
    num_cylinders_ = preset.num_cylinders;
    for (int i = 0; i < MAX_CYLINDERS; ++i) {
        phase_offsets_[i] = preset.phase_offsets[i];
    }
    reset();
}

void CylinderBank::reset() {
    crank_phase_ = 0.0f;
    for (int i = 0; i < MAX_CYLINDERS; ++i) {
        cylinders_[i].phase = phase_offsets_[i];
        cylinders_[i].last_phase = phase_offsets_[i];
        cylinders_[i].impulse_time = 1.0f;
    }
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
                cyl.impulse_time = 0.0f;
            }
            cyl.last_phase = cyl_phase;

            // Generate impulse if active
            if (cyl.impulse_time < duration) {
                float t = cyl.impulse_time;
                float env = std::sin(PI * t / duration) * std::exp(-decay * t);
                sample_val += amplitude * env;
            }

            cyl.impulse_time += dt;
        }

        output[f] += sample_val;
        crank_phase_ += phase_inc;
        if (crank_phase_ >= TWO_PI) {
            crank_phase_ -= TWO_PI;
        }
    }
}

} // namespace engin
