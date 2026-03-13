#pragma once

#include <cstdint>

#ifdef __cplusplus
inline constexpr float    PI           = 3.14159265358979323846f;
inline constexpr float    TWO_PI       = 6.28318530717958647692f;
inline constexpr int      MAX_CYLINDERS = 12;
inline constexpr int      MAX_HARMONICS = 32;
inline constexpr int      MAX_VOICES    = 16;
inline constexpr int      MAX_MIXER_CHANNELS = 8;
inline constexpr int      MAX_EFFECTS   = 8;
inline constexpr int      MAX_EXHAUST_RESONANCES = 3;
inline constexpr uint32_t DEFAULT_SAMPLE_RATE   = 48000;
inline constexpr uint32_t DEFAULT_BUFFER_FRAMES = 512;
inline constexpr int      TRIGGER_QUEUE_SIZE = 64;

/* Bring constants into engin namespace for internal C++ code */
namespace engin {
    using ::PI;
    using ::TWO_PI;
    using ::MAX_CYLINDERS;
    using ::MAX_HARMONICS;
    using ::MAX_VOICES;
    using ::MAX_MIXER_CHANNELS;
    using ::MAX_EFFECTS;
    using ::MAX_EXHAUST_RESONANCES;
    using ::DEFAULT_SAMPLE_RATE;
    using ::DEFAULT_BUFFER_FRAMES;
    using ::TRIGGER_QUEUE_SIZE;
} // namespace engin
#else
#define ENGIN_PI            3.14159265358979323846f
#define ENGIN_TWO_PI        6.28318530717958647692f
#define ENGIN_MAX_CYLINDERS 12
#define ENGIN_MAX_HARMONICS 32
#define ENGIN_MAX_VOICES    16
#define ENGIN_MAX_MIXER_CHANNELS 8
#define ENGIN_MAX_EFFECTS   8
#define ENGIN_MAX_EXHAUST_RESONANCES 3
#define ENGIN_DEFAULT_SAMPLE_RATE   48000
#define ENGIN_DEFAULT_BUFFER_FRAMES 512
#define ENGIN_TRIGGER_QUEUE_SIZE    64
#endif
