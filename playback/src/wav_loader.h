#pragma once

#include "engin/types.h"
#include <vector>

namespace engin {

struct WavData {
    std::vector<float> pcm;
    SampleRate sample_rate = 0;
    uint32_t   num_frames  = 0;
};

// Load a WAV/MP3/FLAC file into raw PCM float data using miniaudio decoder.
// target_rate: desired output sample rate (miniaudio will resample if needed).
// Returns empty WavData on failure.
WavData load_wav(const char* path, SampleRate target_rate);

} // namespace engin
