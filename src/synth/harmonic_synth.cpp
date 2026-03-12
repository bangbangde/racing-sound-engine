#include "synth/harmonic_synth.h"
#include <cmath>
#include <cstring>
#include <algorithm>

namespace engin {

HarmonicSynthesizer::HarmonicSynthesizer() {
    reset();
}

void HarmonicSynthesizer::set_harmonic_profile(const float* amplitudes, int count) {
    num_harmonics_ = std::min(count, MAX_HARMONICS);
    for (int i = 0; i < num_harmonics_; ++i) {
        amplitudes_[i] = amplitudes[i];
    }
    for (int i = num_harmonics_; i < MAX_HARMONICS; ++i) {
        amplitudes_[i] = 0.0f;
    }
}

void HarmonicSynthesizer::reset() {
    std::memset(phases_, 0, sizeof(phases_));
}

void HarmonicSynthesizer::process(Sample* output, FrameCount frames,
                                   float fundamental_freq, float load,
                                   float rpm_normalized,
                                   SampleRate sample_rate) {
    if (num_harmonics_ <= 0 || fundamental_freq <= 0.0f) return;

    const float sr = static_cast<float>(sample_rate);

    for (FrameCount f = 0; f < frames; ++f) {
        float sample_val = 0.0f;

        for (int h = 0; h < num_harmonics_; ++h) {
            // Load modulation: higher harmonics are more affected by load
            float load_mod;
            if (h < 4) {
                load_mod = 0.5f + 0.5f * load; // harmonics 1-4: mild modulation
            } else if (h < 16) {
                load_mod = std::pow(load + 0.1f, 0.5f); // harmonics 5-16: moderate
            } else {
                load_mod = std::pow(load + 0.05f, 1.5f); // harmonics 17+: strong
            }

            // RPM brightness: higher RPM reveals more high-frequency content
            float rpm_brightness;
            if (h < 4) {
                rpm_brightness = 1.0f; // low harmonics unaffected
            } else if (h < 12) {
                rpm_brightness = 0.7f + 0.3f * rpm_normalized; // mid harmonics
            } else {
                rpm_brightness = 0.3f + 0.7f * rpm_normalized; // high harmonics
            }

            float amp = amplitudes_[h] * load_mod * rpm_brightness;
            sample_val += amp * std::sin(phases_[h]);

            // Advance phase
            float harmonic_freq = fundamental_freq * static_cast<float>(h + 1);
            phases_[h] += TWO_PI * harmonic_freq / sr;
            phases_[h] = std::fmod(phases_[h], TWO_PI);
        }

        output[f] += sample_val;
    }
}

} // namespace engin
