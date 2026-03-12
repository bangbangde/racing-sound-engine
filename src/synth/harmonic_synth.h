#pragma once

#include "core/types.h"
#include "core/constants.h"

namespace engin {

class HarmonicSynthesizer {
public:
    HarmonicSynthesizer();

    void set_harmonic_profile(const float* amplitudes, int count);

    // Additive synthesis: generate harmonics of fundamental_freq, modulated by load.
    // Output is additive (accumulated into output buffer).
    void process(Sample* output, FrameCount frames, float fundamental_freq,
                 float load, SampleRate sample_rate);

    void reset();

private:
    float amplitudes_[MAX_HARMONICS] = {};
    float phases_[MAX_HARMONICS]     = {};
    int   num_harmonics_             = 0;
};

} // namespace engin
