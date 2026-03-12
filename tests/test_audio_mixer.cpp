#include "mixer/audio_mixer.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace engin;

void test_single_channel_passthrough() {
    AudioMixer mixer;

    constexpr FrameCount frames = 256;
    Sample source[frames];
    for (FrameCount i = 0; i < frames; ++i) source[i] = 0.5f;

    mixer.mix_into(0, source, frames);

    Sample output[frames] = {};
    mixer.render(output, frames);

    for (FrameCount i = 0; i < frames; ++i) {
        assert(std::fabs(output[i] - 0.5f) < 1e-6f);
    }
    std::printf("  [PASS] single channel passthrough\n");
}

void test_multi_channel_mixing() {
    AudioMixer mixer;

    constexpr FrameCount frames = 256;
    Sample src1[frames], src2[frames];
    for (FrameCount i = 0; i < frames; ++i) {
        src1[i] = 0.3f;
        src2[i] = 0.2f;
    }

    mixer.mix_into(0, src1, frames);
    mixer.mix_into(1, src2, frames);

    Sample output[frames] = {};
    mixer.render(output, frames);

    for (FrameCount i = 0; i < frames; ++i) {
        assert(std::fabs(output[i] - 0.5f) < 1e-5f);
    }
    std::printf("  [PASS] multi-channel mixing: 0.3 + 0.2 = 0.5\n");
}

void test_gain_control() {
    AudioMixer mixer;
    mixer.set_channel_gain(0, 0.5f);

    constexpr FrameCount frames = 256;
    Sample source[frames];
    for (FrameCount i = 0; i < frames; ++i) source[i] = 1.0f;

    mixer.mix_into(0, source, frames);

    Sample output[frames] = {};
    mixer.render(output, frames);

    for (FrameCount i = 0; i < frames; ++i) {
        assert(std::fabs(output[i] - 0.5f) < 1e-5f);
    }
    std::printf("  [PASS] gain control: 1.0 * 0.5 = 0.5\n");
}

void test_mute() {
    AudioMixer mixer;
    mixer.set_channel_mute(0, true);

    constexpr FrameCount frames = 256;
    Sample source[frames];
    for (FrameCount i = 0; i < frames; ++i) source[i] = 1.0f;

    mixer.mix_into(0, source, frames);

    Sample output[frames] = {};
    mixer.render(output, frames);

    for (FrameCount i = 0; i < frames; ++i) {
        assert(std::fabs(output[i]) < 1e-6f);
    }
    std::printf("  [PASS] muted channel = silence\n");
}

int main() {
    std::printf("=== AudioMixer Tests ===\n");
    test_single_channel_passthrough();
    test_multi_channel_mixing();
    test_gain_control();
    test_mute();
    std::printf("All AudioMixer tests passed.\n\n");
    return 0;
}
