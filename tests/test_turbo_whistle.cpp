#include "synth/turbo_whistle.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace engin;

void test_turbo_produces_output() {
    TurboWhistle turbo;
    turbo.set_params(2000.0f, 5000.0f, 0.3f, 0.1f);

    constexpr FrameCount frames = 4096;
    Sample buffer[frames] = {};

    // High RPM + high throttle should produce output after spool-up
    turbo.process(buffer, frames, 0.8f, 0.9f, 48000);

    float max_val = 0.0f;
    for (FrameCount i = 0; i < frames; ++i) {
        float v = std::fabs(buffer[i]);
        if (v > max_val) max_val = v;
    }
    assert(max_val > 0.001f);
    std::printf("  [PASS] turbo output: peak = %.4f\n", max_val);
}

void test_turbo_silent_when_idle() {
    TurboWhistle turbo;
    turbo.set_params(2000.0f, 5000.0f, 0.3f, 0.1f);

    constexpr FrameCount frames = 512;
    Sample buffer[frames] = {};

    // Zero RPM and zero throttle should produce no output
    turbo.process(buffer, frames, 0.0f, 0.0f, 48000);

    float max_val = 0.0f;
    for (FrameCount i = 0; i < frames; ++i) {
        float v = std::fabs(buffer[i]);
        if (v > max_val) max_val = v;
    }
    assert(max_val < 0.001f);
    std::printf("  [PASS] turbo silent at idle: peak = %.6f\n", max_val);
}

void test_turbo_louder_at_high_spool() {
    TurboWhistle turbo;
    turbo.set_params(2000.0f, 5000.0f, 0.3f, 0.1f);

    constexpr FrameCount frames = 4096;
    Sample buf_low[frames] = {};
    Sample buf_high[frames] = {};

    turbo.reset();
    turbo.process(buf_low, frames, 0.3f, 0.3f, 48000);

    turbo.reset();
    turbo.process(buf_high, frames, 0.9f, 0.9f, 48000);

    float rms_low = 0.0f, rms_high = 0.0f;
    for (FrameCount i = 0; i < frames; ++i) {
        rms_low  += buf_low[i] * buf_low[i];
        rms_high += buf_high[i] * buf_high[i];
    }
    rms_low  = std::sqrt(rms_low / frames);
    rms_high = std::sqrt(rms_high / frames);

    assert(rms_high > rms_low);
    std::printf("  [PASS] turbo spool loudness: low=%.4f, high=%.4f\n", rms_low, rms_high);
}

int main() {
    std::printf("=== TurboWhistle Tests ===\n");
    test_turbo_produces_output();
    test_turbo_silent_when_idle();
    test_turbo_louder_at_high_spool();
    std::printf("All TurboWhistle tests passed.\n\n");
    return 0;
}
