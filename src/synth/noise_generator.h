#pragma once

#include "core/types.h"
#include "core/constants.h"

namespace engin {

class NoiseGenerator {
public:
    NoiseGenerator();

    void set_params(float center_freq, float bandwidth, float amplitude);

    // Generate band-limited noise, additive into output buffer
    void process(Sample* output, FrameCount frames, float amplitude_mod,
                 SampleRate sample_rate);

    void reset();

private:
    float center_freq_ = 2000.0f;
    float bandwidth_   = 1000.0f;
    float amplitude_   = 0.1f;

    // xorshift128 state
    uint32_t rng_state_[4] = {123456789, 362436069, 521288629, 88675123};
    float xorshift128();

    // Simple one-pole bandpass state
    float bp_x1_ = 0.0f;
    float bp_x2_ = 0.0f;
    float bp_y1_ = 0.0f;
    float bp_y2_ = 0.0f;
    float bp_b0_ = 0.0f, bp_b1_ = 0.0f, bp_b2_ = 0.0f;
    float bp_a1_ = 0.0f, bp_a2_ = 0.0f;

    void update_filter_coeffs(SampleRate sample_rate);
};

} // namespace engin
