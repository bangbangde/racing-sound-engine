#pragma once

#include "engin/types.h"
#include <atomic>

namespace engin {

class SynthEngine;

class AudioPlayback {
public:
    AudioPlayback();
    ~AudioPlayback();

    bool init(SynthEngine& synth, SampleRate sample_rate, FrameCount buffer_frames);
    void start();
    void stop();
    void shutdown();

    bool is_running() const { return running_.load(std::memory_order_relaxed); }

private:
    struct MaDeviceWrapper;
    MaDeviceWrapper* device_ = nullptr;
    SynthEngine* synth_ = nullptr;
    std::atomic<bool> running_{false};
};

} // namespace engin
