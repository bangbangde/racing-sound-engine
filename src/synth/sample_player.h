#pragma once

#include "core/types.h"
#include "core/constants.h"
#include <atomic>
#include <vector>

namespace engin {

struct TriggerEvent {
    int   sample_id   = -1;
    float gain        = 1.0f;
    float pitch_ratio = 1.0f;
};

class SamplePlayer {
public:
    SamplePlayer();

    // Load a WAV sample (call during init, NOT real-time safe)
    int load_sample(const char* path, bool loop = false);

    // Trigger sample playback (call from control thread; uses lock-free queue)
    void trigger(int sample_id, float gain = 1.0f, float pitch_ratio = 1.0f);

    // Stop all voices
    void stop_all();

    // Process and mix active voices into output (call from audio thread)
    void process(Sample* output, FrameCount frames);

    void set_sample_rate(SampleRate sr) { sample_rate_ = sr; }

private:
    struct LoadedSample {
        std::vector<Sample> data;
        SampleRate rate = 48000;
        bool loop       = false;
    };

    struct ActiveVoice {
        int   sample_id   = -1;
        float position    = 0.0f;   // fractional sample position
        float gain        = 1.0f;
        float pitch_ratio = 1.0f;
        bool  active      = false;
    };

    std::vector<LoadedSample> samples_;
    ActiveVoice voices_[MAX_VOICES] = {};
    SampleRate sample_rate_ = DEFAULT_SAMPLE_RATE;

    // SPSC lock-free trigger queue
    TriggerEvent trigger_queue_[TRIGGER_QUEUE_SIZE] = {};
    std::atomic<int> queue_write_{0};
    std::atomic<int> queue_read_{0};

    void drain_trigger_queue();
};

} // namespace engin
