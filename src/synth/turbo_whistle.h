#pragma once

#include "core/types.h"
#include "core/constants.h"

namespace engin {

class TurboWhistle {
public:
    TurboWhistle();

    void set_params(float base_freq, float freq_range, float noise_mix, float max_amplitude);

    // Generate turbo whistle signal, additive into output buffer.
    // rpm_normalized: 0.0 (idle) to 1.0 (redline)
    // throttle: 0.0 to 1.0
    void process(Sample* output, FrameCount frames, float rpm_normalized,
                 float throttle, SampleRate sample_rate);

    void reset();

private:
    float base_freq_     = 2000.0f;  // turbo min frequency (Hz)
    float freq_range_    = 5000.0f;  // frequency sweep range (Hz)
    float noise_mix_     = 0.3f;     // noise vs tone mix (0=pure tone, 1=pure noise)
    float max_amplitude_ = 0.08f;    // maximum output amplitude

    float phase_         = 0.0f;     // sine oscillator phase
    float spool_         = 0.0f;     // turbo spool state (0~1), one-pole smoothed

    // xorshift128 RNG state for noise and FM jitter
    uint32_t rng_state_[4] = {98765432, 456789012, 321654987, 112233445};
    float xorshift128();

    // Bandpass filter state for noise shaping
    float bp_y1_ = 0.0f;
    float bp_y2_ = 0.0f;
};

} // namespace engin
