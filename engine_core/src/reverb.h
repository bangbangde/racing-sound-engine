#pragma once

#include "biquad_filter.h"
#include <array>

namespace engin {

// Schroeder reverb: 4 comb filters + 2 allpass filters
class SimpleReverb : public IEffect {
public:
    SimpleReverb();

    void set_params(float room_size, float damping, float mix);

    void process(Sample* buffer, FrameCount frames) override;
    void reset() override;

private:
    float room_size_ = 0.5f;
    float damping_   = 0.5f;
    float mix_       = 0.2f;

    // Comb filter
    static constexpr int NUM_COMBS = 4;
    static constexpr int COMB_SIZES[NUM_COMBS] = {1116, 1188, 1277, 1356};
    struct CombFilter {
        static constexpr int MAX_SIZE = 2048;
        float buffer[MAX_SIZE] = {};
        int   size   = 1116;
        int   pos    = 0;
        float filter = 0.0f;

        float process(float input, float feedback, float damp) {
            float out = buffer[pos];
            filter = out * (1.0f - damp) + filter * damp;
            buffer[pos] = input + filter * feedback;
            pos = (pos + 1) % size;
            return out;
        }

        void clear() {
            for (int i = 0; i < MAX_SIZE; ++i) buffer[i] = 0.0f;
            pos = 0;
            filter = 0.0f;
        }
    };

    // Allpass filter
    static constexpr int NUM_ALLPASSES = 2;
    static constexpr int ALLPASS_SIZES[NUM_ALLPASSES] = {556, 441};
    struct AllpassFilter {
        static constexpr int MAX_SIZE = 1024;
        float buffer[MAX_SIZE] = {};
        int   size = 556;
        int   pos  = 0;

        float process(float input, float feedback = 0.5f) {
            float delayed = buffer[pos];
            float out = -input + delayed;
            buffer[pos] = input + delayed * feedback;
            pos = (pos + 1) % size;
            return out;
        }

        void clear() {
            for (int i = 0; i < MAX_SIZE; ++i) buffer[i] = 0.0f;
            pos = 0;
        }
    };

    std::array<CombFilter, NUM_COMBS>       combs_;
    std::array<AllpassFilter, NUM_ALLPASSES> allpasses_;
};

} // namespace engin
