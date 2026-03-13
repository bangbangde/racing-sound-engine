#include "engin/engin_core.h"
#include "synth_engine.h"

using namespace engin;

extern "C" {

EnginHandle engin_create(unsigned int sample_rate) {
    auto* engine = new SynthEngine();
    engine->init(static_cast<SampleRate>(sample_rate));
    return static_cast<EnginHandle>(engine);
}

void engin_destroy(EnginHandle handle) {
    delete static_cast<SynthEngine*>(handle);
}

void engin_load_preset(EnginHandle handle, const EnginePreset* preset) {
    if (handle && preset) {
        static_cast<SynthEngine*>(handle)->load_preset(*preset);
    }
}

int engin_load_sample(EnginHandle handle, const float* pcm_data,
                       unsigned int num_frames, unsigned int original_rate,
                       int loop) {
    if (!handle) return -1;
    return static_cast<SynthEngine*>(handle)->load_sample(
        pcm_data, num_frames, static_cast<SampleRate>(original_rate), loop != 0);
}

void engin_set_params(EnginHandle handle, const EngineParams* params) {
    if (handle && params) {
        static_cast<SynthEngine*>(handle)->set_params(*params);
    }
}

void engin_process(EnginHandle handle, float* output, unsigned int num_frames) {
    if (handle && output) {
        static_cast<SynthEngine*>(handle)->process(output, static_cast<FrameCount>(num_frames));
    }
}

void engin_reset(EnginHandle handle) {
    if (handle) {
        static_cast<SynthEngine*>(handle)->reset();
    }
}

} // extern "C"
