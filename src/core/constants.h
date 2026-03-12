#pragma once

#include <cmath>
#include <cstdint>

namespace engin {

inline constexpr float  PI           = 3.14159265358979323846f;
inline constexpr float  TWO_PI       = 6.28318530717958647692f;
inline constexpr int    MAX_CYLINDERS = 12;
inline constexpr int    MAX_HARMONICS = 32;
inline constexpr int    MAX_VOICES    = 16;
inline constexpr int    MAX_MIXER_CHANNELS = 8;
inline constexpr int    MAX_EFFECTS   = 8;
inline constexpr uint32_t DEFAULT_SAMPLE_RATE   = 48000;
inline constexpr uint32_t DEFAULT_BUFFER_FRAMES = 512;
inline constexpr int    TRIGGER_QUEUE_SIZE = 64;

} // namespace engin
