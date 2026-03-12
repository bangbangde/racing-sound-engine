#include "synth/harmonic_synth.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace engin;

void test_produces_output() {
    HarmonicSynthesizer synth;
    float amps[] = {1.0f, 0.5f, 0.25f, 0.125f};
    synth.set_harmonic_profile(amps, 4);

    constexpr FrameCount frames = 2048;
    Sample buffer[frames] = {};

    synth.process(buffer, frames, 440.0f, 0.5f, 48000);

    float max_val = 0.0f;
    for (FrameCount i = 0; i < frames; ++i) {
        float v = std::fabs(buffer[i]);
        if (v > max_val) max_val = v;
    }
    assert(max_val > 0.1f);
    std::printf("  [PASS] harmonic output: peak = %.4f\n", max_val);
}

void test_load_modulates_harmonics() {
    HarmonicSynthesizer synth;
    float amps[MAX_HARMONICS];
    for (int i = 0; i < MAX_HARMONICS; ++i) amps[i] = 0.1f;
    synth.set_harmonic_profile(amps, 24);

    constexpr FrameCount frames = 2048;
    Sample buf_low[frames] = {};
    Sample buf_high[frames] = {};

    synth.reset();
    synth.process(buf_low, frames, 200.0f, 0.1f, 48000);

    synth.reset();
    synth.process(buf_high, frames, 200.0f, 1.0f, 48000);

    float rms_low = 0.0f, rms_high = 0.0f;
    for (FrameCount i = 0; i < frames; ++i) {
        rms_low  += buf_low[i] * buf_low[i];
        rms_high += buf_high[i] * buf_high[i];
    }
    rms_low  = std::sqrt(rms_low / frames);
    rms_high = std::sqrt(rms_high / frames);

    assert(rms_high > rms_low);
    std::printf("  [PASS] load modulation: low_load=%.4f, high_load=%.4f\n", rms_low, rms_high);
}

void test_zero_harmonics_silent() {
    HarmonicSynthesizer synth;

    constexpr FrameCount frames = 512;
    Sample buffer[frames] = {};

    synth.process(buffer, frames, 440.0f, 0.5f, 48000);

    float max_val = 0.0f;
    for (FrameCount i = 0; i < frames; ++i) {
        float v = std::fabs(buffer[i]);
        if (v > max_val) max_val = v;
    }
    assert(max_val < 0.0001f);
    std::printf("  [PASS] zero harmonics: silence (peak = %.6f)\n", max_val);
}

int main() {
    std::printf("=== HarmonicSynthesizer Tests ===\n");
    test_produces_output();
    test_load_modulates_harmonics();
    test_zero_harmonics_silent();
    std::printf("All HarmonicSynthesizer tests passed.\n\n");
    return 0;
}
