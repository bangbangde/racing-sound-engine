#include "reverb.h"
#include <algorithm>

namespace engin {

SimpleReverb::SimpleReverb() {
    for (int i = 0; i < NUM_COMBS; ++i) {
        combs_[i].size = COMB_SIZES[i];
    }
    for (int i = 0; i < NUM_ALLPASSES; ++i) {
        allpasses_[i].size = ALLPASS_SIZES[i];
    }
    reset();
}

void SimpleReverb::set_params(float room_size, float damping, float mix) {
    room_size_ = std::clamp(room_size, 0.0f, 1.0f);
    damping_   = std::clamp(damping, 0.0f, 1.0f);
    mix_       = std::clamp(mix, 0.0f, 1.0f);
}

void SimpleReverb::process(Sample* buffer, FrameCount frames) {
    float feedback = room_size_ * 0.85f + 0.1f; // map to 0.1~0.95

    for (FrameCount i = 0; i < frames; ++i) {
        float dry = buffer[i];
        float wet = 0.0f;

        // Sum of 4 comb filters in parallel
        for (auto& comb : combs_) {
            wet += comb.process(dry, feedback, damping_);
        }
        wet *= 0.25f; // normalize

        // Series allpass filters
        for (auto& ap : allpasses_) {
            wet = ap.process(wet);
        }

        buffer[i] = dry * (1.0f - mix_) + wet * mix_;
    }
}

void SimpleReverb::reset() {
    for (auto& c : combs_) c.clear();
    for (auto& a : allpasses_) a.clear();
}

} // namespace engin
