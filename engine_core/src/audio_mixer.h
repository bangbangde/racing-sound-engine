#pragma once

#include "engin/types.h"
#include "engin/constants.h"
#include <cstring>

namespace engin {

class AudioMixer {
public:
    AudioMixer();

    void set_channel_gain(int channel, float gain);
    void set_channel_mute(int channel, bool mute);

    // Accumulate source into a channel buffer
    void mix_into(int channel, const Sample* source, FrameCount frames);

    // Render all channels into output, then clear channel buffers
    void render(Sample* output, FrameCount frames);

    void clear_all();

private:
    static constexpr FrameCount MAX_BUFFER_SIZE = 4096;

    struct Channel {
        Sample buffer[MAX_BUFFER_SIZE] = {};
        float  gain  = 1.0f;
        bool   muted = false;
    };

    Channel channels_[MAX_MIXER_CHANNELS] = {};
};

} // namespace engin
