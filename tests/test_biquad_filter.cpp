#include "effects/biquad_filter.h"
#include "core/constants.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace engin;

// Generate a sine wave at a given frequency
void gen_sine(Sample* buf, FrameCount frames, float freq, SampleRate sr) {
    for (FrameCount i = 0; i < frames; ++i) {
        buf[i] = std::sin(TWO_PI * freq * static_cast<float>(i) / static_cast<float>(sr));
    }
}

float compute_rms(const Sample* buf, FrameCount frames) {
    float sum = 0.0f;
    for (FrameCount i = 0; i < frames; ++i) {
        sum += buf[i] * buf[i];
    }
    return std::sqrt(sum / static_cast<float>(frames));
}

void test_lowpass_attenuates_high() {
    BiquadFilter filt;
    filt.set_params(FilterType::LowPass, 1000.0f, 0.707f, 0.0f, 48000);

    constexpr FrameCount frames = 4096;

    // 200 Hz should pass through mostly
    Sample buf_low[frames];
    gen_sine(buf_low, frames, 200.0f, 48000);
    float rms_before_low = compute_rms(buf_low, frames);
    filt.reset();
    filt.process(buf_low, frames);
    float rms_after_low = compute_rms(buf_low, frames);

    // 5000 Hz should be attenuated
    Sample buf_high[frames];
    gen_sine(buf_high, frames, 5000.0f, 48000);
    float rms_before_high = compute_rms(buf_high, frames);
    filt.reset();
    filt.process(buf_high, frames);
    float rms_after_high = compute_rms(buf_high, frames);

    float ratio_low = rms_after_low / rms_before_low;
    float ratio_high = rms_after_high / rms_before_high;

    assert(ratio_low > 0.5f);     // low freq passes through
    assert(ratio_high < 0.2f);    // high freq attenuated
    std::printf("  [PASS] LP filter: 200Hz ratio=%.3f, 5kHz ratio=%.3f\n", ratio_low, ratio_high);
}

void test_highpass_attenuates_low() {
    BiquadFilter filt;
    filt.set_params(FilterType::HighPass, 1000.0f, 0.707f, 0.0f, 48000);

    constexpr FrameCount frames = 4096;

    // 100 Hz should be attenuated
    Sample buf_low[frames];
    gen_sine(buf_low, frames, 100.0f, 48000);
    float rms_before = compute_rms(buf_low, frames);
    filt.reset();
    filt.process(buf_low, frames);
    float rms_after = compute_rms(buf_low, frames);

    float ratio = rms_after / rms_before;
    assert(ratio < 0.2f);
    std::printf("  [PASS] HP filter: 100Hz ratio=%.3f\n", ratio);
}

void test_passthrough_identity() {
    // With extremely high cutoff LP, signal should pass nearly unchanged
    BiquadFilter filt;
    filt.set_params(FilterType::LowPass, 20000.0f, 0.707f, 0.0f, 48000);

    constexpr FrameCount frames = 2048;
    Sample buf[frames];
    gen_sine(buf, frames, 440.0f, 48000);
    float rms_before = compute_rms(buf, frames);

    filt.process(buf, frames);
    float rms_after = compute_rms(buf, frames);

    float ratio = rms_after / rms_before;
    assert(ratio > 0.9f && ratio < 1.1f);
    std::printf("  [PASS] near-passthrough: ratio=%.3f\n", ratio);
}

int main() {
    std::printf("=== BiquadFilter Tests ===\n");
    test_lowpass_attenuates_high();
    test_highpass_attenuates_low();
    test_passthrough_identity();
    std::printf("All BiquadFilter tests passed.\n\n");
    return 0;
}
