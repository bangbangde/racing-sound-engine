#include "synth/noise_generator.h"
#include <cmath>
#include <algorithm>

namespace engin {

NoiseGenerator::NoiseGenerator() {
    reset();
}

void NoiseGenerator::set_params(float center_freq, float bandwidth, float amplitude) {
    center_freq_ = center_freq;
    bandwidth_   = bandwidth;
    amplitude_   = amplitude;
}

void NoiseGenerator::reset() {
    bp_x1_ = bp_x2_ = bp_y1_ = bp_y2_ = 0.0f;
    rng_state_[0] = 123456789;
    rng_state_[1] = 362436069;
    rng_state_[2] = 521288629;
    rng_state_[3] = 88675123;
}

float NoiseGenerator::xorshift128() {
    uint32_t t = rng_state_[3];
    t ^= t << 11;
    t ^= t >> 8;
    rng_state_[3] = rng_state_[2];
    rng_state_[2] = rng_state_[1];
    rng_state_[1] = rng_state_[0];
    t ^= rng_state_[0];
    t ^= rng_state_[0] >> 19;
    rng_state_[0] = t;
    // Convert to float in [-1, 1]
    return static_cast<float>(static_cast<int32_t>(t)) / 2147483648.0f;
}

void NoiseGenerator::update_filter_coeffs(SampleRate sample_rate) {
    float sr = static_cast<float>(sample_rate);
    float w0 = TWO_PI * center_freq_ / sr;
    float Q  = center_freq_ / std::max(bandwidth_, 1.0f);
    float alpha = std::sin(w0) / (2.0f * Q);

    float b0 = alpha;
    float b1 = 0.0f;
    float b2 = -alpha;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * std::cos(w0);
    float a2 = 1.0f - alpha;

    // Normalize
    bp_b0_ = b0 / a0;
    bp_b1_ = b1 / a0;
    bp_b2_ = b2 / a0;
    bp_a1_ = a1 / a0;
    bp_a2_ = a2 / a0;
}

void NoiseGenerator::process(Sample* output, FrameCount frames,
                              float amplitude_mod, SampleRate sample_rate) {
    update_filter_coeffs(sample_rate);

    float gain = amplitude_ * amplitude_mod;
    for (FrameCount f = 0; f < frames; ++f) {
        float x = xorshift128();

        // Direct Form II Transposed biquad bandpass
        float y = bp_b0_ * x + bp_b1_ * bp_x1_ + bp_b2_ * bp_x2_
                  - bp_a1_ * bp_y1_ - bp_a2_ * bp_y2_;

        bp_x2_ = bp_x1_;
        bp_x1_ = x;
        bp_y2_ = bp_y1_;
        bp_y1_ = y;

        output[f] += gain * y;
    }
}

} // namespace engin
