#include "sample_player.h"
#include <cmath>
#include <cstring>

namespace engin {

SamplePlayer::SamplePlayer() = default;

int SamplePlayer::load_sample(const float* pcm_data, uint32_t num_frames,
                               SampleRate original_rate, bool loop) {
    if (!pcm_data || num_frames == 0) return -1;

    LoadedSample sample;
    sample.data.assign(pcm_data, pcm_data + num_frames);
    sample.rate = original_rate;
    sample.loop = loop;

    int id = static_cast<int>(samples_.size());
    samples_.push_back(std::move(sample));
    return id;
}

void SamplePlayer::trigger(int sample_id, float gain, float pitch_ratio) {
    int w = queue_write_.load(std::memory_order_relaxed);
    int next_w = (w + 1) % TRIGGER_QUEUE_SIZE;
    if (next_w == queue_read_.load(std::memory_order_acquire)) {
        return; // queue full, drop event
    }
    trigger_queue_[w] = {sample_id, gain, pitch_ratio};
    queue_write_.store(next_w, std::memory_order_release);
}

void SamplePlayer::stop_all() {
    for (auto& v : voices_) {
        v.active = false;
    }
}

void SamplePlayer::drain_trigger_queue() {
    while (true) {
        int r = queue_read_.load(std::memory_order_relaxed);
        if (r == queue_write_.load(std::memory_order_acquire)) break;

        TriggerEvent& evt = trigger_queue_[r];
        if (evt.sample_id >= 0 && evt.sample_id < static_cast<int>(samples_.size())) {
            // Find a free voice
            for (auto& v : voices_) {
                if (!v.active) {
                    v.sample_id   = evt.sample_id;
                    v.position    = 0.0f;
                    v.gain        = evt.gain;
                    v.pitch_ratio = evt.pitch_ratio;
                    v.active      = true;
                    break;
                }
            }
        }
        queue_read_.store((r + 1) % TRIGGER_QUEUE_SIZE, std::memory_order_release);
    }
}

void SamplePlayer::process(Sample* output, FrameCount frames) {
    drain_trigger_queue();

    for (auto& v : voices_) {
        if (!v.active) continue;

        const auto& sample = samples_[v.sample_id];
        const auto sample_len = static_cast<float>(sample.data.size());

        for (FrameCount f = 0; f < frames; ++f) {
            float pos = v.position;
            if (pos >= sample_len) {
                if (sample.loop) {
                    pos = std::fmod(pos, sample_len);
                    v.position = pos;
                } else {
                    v.active = false;
                    break;
                }
            }

            // Linear interpolation
            int   idx0 = static_cast<int>(pos);
            int   idx1 = idx0 + 1;
            float frac = pos - static_cast<float>(idx0);

            float s0 = sample.data[idx0];
            float s1 = (idx1 < static_cast<int>(sample.data.size())) ? sample.data[idx1] : s0;
            float val = s0 + frac * (s1 - s0);

            output[f] += val * v.gain;
            v.position += v.pitch_ratio;
        }
    }
}

} // namespace engin
