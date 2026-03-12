#pragma once

#include "effects/biquad_filter.h"

namespace engin {

class Distortion : public IEffect {
public:
    Distortion();

    void set_params(float drive, float mix);

    void process(Sample* buffer, FrameCount frames) override;
    void reset() override;

private:
    float drive_ = 2.0f;
    float mix_   = 0.5f;
};

} // namespace engin
