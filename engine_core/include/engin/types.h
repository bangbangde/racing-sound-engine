#pragma once

#include <cstdint>

typedef float    Sample;
typedef uint32_t FrameCount;
typedef uint32_t SampleRate;

/* Shift state machine phases — plain enum for C compatibility */
enum ShiftPhase {
    ShiftPhase_Idle            = 0,
    ShiftPhase_ClutchDisengage = 1,
    ShiftPhase_GearChange      = 2,
    ShiftPhase_ClutchEngage    = 3,
};

/* Bring types into engin namespace for internal C++ code */
#ifdef __cplusplus
namespace engin {
    using ::Sample;
    using ::FrameCount;
    using ::SampleRate;
    using ::ShiftPhase;
} // namespace engin
#endif
