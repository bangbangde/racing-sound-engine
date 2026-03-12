#include "mixer/audio_mixer.h"
#include <algorithm>

namespace engin {

AudioMixer::AudioMixer() {
    clear_all();
}

void AudioMixer::set_channel_gain(int channel, float gain) {
    if (channel >= 0 && channel < MAX_MIXER_CHANNELS) {
        channels_[channel].gain = gain;
    }
}

void AudioMixer::set_channel_mute(int channel, bool mute) {
    if (channel >= 0 && channel < MAX_MIXER_CHANNELS) {
        channels_[channel].muted = mute;
    }
}

void AudioMixer::mix_into(int channel, const Sample* source, FrameCount frames) {
    if (channel < 0 || channel >= MAX_MIXER_CHANNELS) return;
    FrameCount n = std::min(frames, static_cast<FrameCount>(MAX_BUFFER_SIZE));
    Sample* buf = channels_[channel].buffer;
    for (FrameCount i = 0; i < n; ++i) {
        buf[i] += source[i];
    }
}

void AudioMixer::render(Sample* output, FrameCount frames) {
    FrameCount n = std::min(frames, static_cast<FrameCount>(MAX_BUFFER_SIZE));
    std::memset(output, 0, n * sizeof(Sample));

    for (int ch = 0; ch < MAX_MIXER_CHANNELS; ++ch) {
        auto& c = channels_[ch];
        if (c.muted) {
            std::memset(c.buffer, 0, n * sizeof(Sample));
            continue;
        }
        float g = c.gain;
        for (FrameCount i = 0; i < n; ++i) {
            output[i] += c.buffer[i] * g;
        }
        std::memset(c.buffer, 0, n * sizeof(Sample));
    }
}

void AudioMixer::clear_all() {
    for (auto& ch : channels_) {
        std::memset(ch.buffer, 0, sizeof(ch.buffer));
        ch.gain  = 1.0f;
        ch.muted = false;
    }
}

} // namespace engin
