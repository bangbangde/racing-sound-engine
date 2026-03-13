#include "distortion.h"
#include <cmath>
#include <algorithm>

namespace engin {

Distortion::Distortion() = default;

void Distortion::set_params(float drive, float mix) {
    drive_ = std::max(0.1f, drive);
    mix_   = std::clamp(mix, 0.0f, 1.0f);
}

void Distortion::process(Sample* buffer, FrameCount frames) {
    for (FrameCount i = 0; i < frames; ++i) {
        float dry = buffer[i];
        float wet = std::tanh(drive_ * dry);
        buffer[i] = mix_ * wet + (1.0f - mix_) * dry;
    }
}

void Distortion::reset() {
    // Stateless effect, nothing to reset
}

} // namespace engin
