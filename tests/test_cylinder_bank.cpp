#include "core/cylinder_bank.h"
#include "presets/engine_preset.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace engin;

void test_i4_produces_output() {
    CylinderBank bank;
    bank.configure(get_preset_i4());

    constexpr FrameCount frames = 1024;
    Sample buffer[frames] = {};
    float firing_freq = (3000.0f * 4) / (60.0f * 2.0f); // 100 Hz
    bank.process(buffer, frames, firing_freq, 0.5f, 48000);

    // Check that output is not all zeros
    float max_val = 0.0f;
    for (FrameCount i = 0; i < frames; ++i) {
        float v = std::fabs(buffer[i]);
        if (v > max_val) max_val = v;
    }
    assert(max_val > 0.001f);
    std::printf("  [PASS] I4 output: peak = %.4f\n", max_val);
}

void test_v8_produces_output() {
    CylinderBank bank;
    bank.configure(get_preset_v8_cross());

    constexpr FrameCount frames = 1024;
    Sample buffer[frames] = {};
    float firing_freq = (3000.0f * 8) / (60.0f * 2.0f); // 200 Hz
    bank.process(buffer, frames, firing_freq, 0.5f, 48000);

    float max_val = 0.0f;
    for (FrameCount i = 0; i < frames; ++i) {
        float v = std::fabs(buffer[i]);
        if (v > max_val) max_val = v;
    }
    assert(max_val > 0.001f);
    std::printf("  [PASS] V8 cross output: peak = %.4f\n", max_val);
}

void test_higher_throttle_louder() {
    CylinderBank bank;
    bank.configure(get_preset_i4());

    constexpr FrameCount frames = 2048;
    Sample buf_low[frames] = {};
    Sample buf_high[frames] = {};

    float firing_freq = 100.0f;

    bank.reset();
    bank.process(buf_low, frames, firing_freq, 0.1f, 48000);

    bank.reset();
    bank.process(buf_high, frames, firing_freq, 0.9f, 48000);

    // Compute RMS
    float rms_low = 0.0f, rms_high = 0.0f;
    for (FrameCount i = 0; i < frames; ++i) {
        rms_low  += buf_low[i] * buf_low[i];
        rms_high += buf_high[i] * buf_high[i];
    }
    rms_low  = std::sqrt(rms_low / frames);
    rms_high = std::sqrt(rms_high / frames);

    assert(rms_high > rms_low);
    std::printf("  [PASS] throttle loudness: low=%.4f, high=%.4f\n", rms_low, rms_high);
}

int main() {
    std::printf("=== CylinderBank Tests ===\n");
    test_i4_produces_output();
    test_v8_produces_output();
    test_higher_throttle_louder();
    std::printf("All CylinderBank tests passed.\n\n");
    return 0;
}
