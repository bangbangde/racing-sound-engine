#pragma once

#include "engin/types.h"
#include "engin/constants.h"

namespace engin {

class HarmonicSynthesizer {
public:
    HarmonicSynthesizer();

    void set_harmonic_profile(const float* amplitudes, int count);

    // Additive synthesis: generate harmonics of fundamental_freq, modulated by load and RPM.
    // rpm_normalized: 0.0 (idle) to 1.0 (redline), controls high-frequency brightness.
    // Output is additive (accumulated into output buffer).
    void process(Sample* output, FrameCount frames, float fundamental_freq,
                 float load, float rpm_normalized, SampleRate sample_rate);

    void reset();

private:
    float amplitudes_[MAX_HARMONICS] = {};
    float phases_[MAX_HARMONICS]     = {};
    int   num_harmonics_             = 0;
};

} // namespace engin
