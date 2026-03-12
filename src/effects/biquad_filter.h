#pragma once

#include "core/types.h"

namespace engin {

enum class FilterType {
    LowPass,
    HighPass,
    BandPass,
    Notch,
    Peak,
    LowShelf,
    HighShelf
};

// IEffect interface for effects chain
class IEffect {
public:
    virtual ~IEffect() = default;
    virtual void process(Sample* buffer, FrameCount frames) = 0;
    virtual void reset() = 0;
};

class BiquadFilter : public IEffect {
public:
    BiquadFilter();

    void set_params(FilterType type, float freq, float Q, float gain_db,
                    SampleRate sample_rate);

    void process(Sample* buffer, FrameCount frames) override;
    void reset() override;

private:
    // Coefficients
    float b0_ = 1.0f, b1_ = 0.0f, b2_ = 0.0f;
    float a1_ = 0.0f, a2_ = 0.0f;

    // State (Direct Form II Transposed)
    float z1_ = 0.0f, z2_ = 0.0f;
};

} // namespace engin
