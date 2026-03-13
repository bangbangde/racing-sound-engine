#ifndef ENGIN_CORE_H
#define ENGIN_CORE_H

#include "engin/types.h"
#include "engin/constants.h"
#include "engin/engine_preset.h"
#include "engin/engine_params.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Export macro */
#ifdef _WIN32
  #ifdef ENGIN_BUILDING_DLL
    #define ENGIN_API __declspec(dllexport)
  #else
    #define ENGIN_API
  #endif
#elif defined(__GNUC__) || defined(__clang__)
  #define ENGIN_API __attribute__((visibility("default")))
#else
  #define ENGIN_API
#endif

/* Opaque handle to SynthEngine instance */
typedef void* EnginHandle;

/* Create a new engine instance with the given sample rate. */
ENGIN_API EnginHandle engin_create(unsigned int sample_rate);

/* Destroy an engine instance and free all resources. */
ENGIN_API void engin_destroy(EnginHandle handle);

/* Load an engine preset (configures all synthesis parameters). */
ENGIN_API void engin_load_preset(EnginHandle handle, const struct EnginePreset* preset);

/* Load a raw PCM sample. Returns sample_id (>=0) or -1 on failure.
   pcm_data: mono float array, num_frames: number of samples,
   original_rate: sample rate of the PCM data, loop: 1=loop, 0=one-shot */
ENGIN_API int engin_load_sample(EnginHandle handle, const float* pcm_data,
                                 unsigned int num_frames, unsigned int original_rate,
                                 int loop);

/* Set real-time engine parameters (RPM, throttle, gear, etc.) */
ENGIN_API void engin_set_params(EnginHandle handle, const struct EngineParams* params);

/* Render audio into the output buffer.
   output: float buffer of at least num_frames elements (mono). */
ENGIN_API void engin_process(EnginHandle handle, float* output, unsigned int num_frames);

/* Reset all internal state (filters, phases, oscillators). */
ENGIN_API void engin_reset(EnginHandle handle);

#ifdef __cplusplus
}
#endif

#endif /* ENGIN_CORE_H */
