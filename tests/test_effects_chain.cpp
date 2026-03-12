#include "effects/effects_chain.h"
#include "effects/distortion.h"
#include "effects/reverb.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace engin;

void test_empty_chain_passthrough() {
    EffectsChain chain;

    constexpr FrameCount frames = 512;
    Sample input[frames];
    Sample output[frames];
    for (FrameCount i = 0; i < frames; ++i) {
        input[i] = output[i] = std::sin(6.2832f * 440.0f * static_cast<float>(i) / 48000.0f);
    }

    chain.process(output, frames);

    // Should be identical (empty chain = passthrough)
    for (FrameCount i = 0; i < frames; ++i) {
        assert(std::fabs(input[i] - output[i]) < 1e-6f);
    }
    std::printf("  [PASS] empty chain = passthrough\n");
}

void test_single_effect_chain() {
    EffectsChain chain;
    Distortion dist;
    dist.set_params(5.0f, 1.0f); // heavy distortion, 100% wet
    chain.add_effect(&dist);

    constexpr FrameCount frames = 512;
    Sample buf[frames];
    for (FrameCount i = 0; i < frames; ++i) {
        buf[i] = std::sin(6.2832f * 440.0f * static_cast<float>(i) / 48000.0f);
    }

    chain.process(buf, frames);

    // After distortion, all values should be within [-1, 1] (tanh clipping)
    for (FrameCount i = 0; i < frames; ++i) {
        assert(buf[i] >= -1.0f && buf[i] <= 1.0f);
    }
    std::printf("  [PASS] single effect chain produces bounded output\n");
}

void test_chain_reset() {
    EffectsChain chain;
    SimpleReverb rev;
    rev.set_params(0.9f, 0.5f, 1.0f);
    chain.add_effect(&rev);

    constexpr FrameCount frames = 512;
    Sample buf[frames];
    for (FrameCount i = 0; i < frames; ++i) buf[i] = 1.0f;
    chain.process(buf, frames);

    chain.reset();

    // After reset, processing silence should give near-silence
    Sample silent[frames] = {};
    chain.process(silent, frames);

    float max_val = 0.0f;
    for (FrameCount i = 0; i < frames; ++i) {
        float v = std::fabs(silent[i]);
        if (v > max_val) max_val = v;
    }
    assert(max_val < 0.01f);
    std::printf("  [PASS] chain reset clears state (peak after reset = %.6f)\n", max_val);
}

int main() {
    std::printf("=== EffectsChain Tests ===\n");
    test_empty_chain_passthrough();
    test_single_effect_chain();
    test_chain_reset();
    std::printf("All EffectsChain tests passed.\n\n");
    return 0;
}
