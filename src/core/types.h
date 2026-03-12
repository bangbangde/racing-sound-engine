#pragma once

#include <cstdint>

namespace engin {

using Sample     = float;
using FrameCount = uint32_t;
using SampleRate = uint32_t;

// Shift state machine phases
enum class ShiftPhase {
    Idle,             // No shift in progress
    ClutchDisengage,  // Clutch releasing
    GearChange,       // Gear changing, RPM interpolating to target
    ClutchEngage,     // Clutch re-engaging
};

} // namespace engin
