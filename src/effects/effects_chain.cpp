#include "effects/effects_chain.h"

namespace engin {

EffectsChain::EffectsChain() = default;

void EffectsChain::add_effect(IEffect* effect) {
    if (num_effects_ < MAX_EFFECTS && effect) {
        effects_[num_effects_++] = effect;
    }
}

void EffectsChain::clear() {
    num_effects_ = 0;
}

void EffectsChain::process(Sample* buffer, FrameCount frames) {
    for (int i = 0; i < num_effects_; ++i) {
        effects_[i]->process(buffer, frames);
    }
}

void EffectsChain::reset() {
    for (int i = 0; i < num_effects_; ++i) {
        effects_[i]->reset();
    }
}

} // namespace engin
