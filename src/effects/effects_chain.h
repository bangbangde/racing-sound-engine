#pragma once

#include "effects/biquad_filter.h"
#include "core/constants.h"

namespace engin {

class EffectsChain : public IEffect {
public:
    EffectsChain();

    void add_effect(IEffect* effect);
    void clear();

    void process(Sample* buffer, FrameCount frames) override;
    void reset() override;

private:
    IEffect* effects_[MAX_EFFECTS] = {};
    int      num_effects_ = 0;
};

} // namespace engin
