#pragma once

#include "engin/types.h"

struct EngineParams {
    float rpm;
    float throttle;
    int   gear;          /* 0=neutral, 1-6 */
    float clutch;        /* 0=disengaged, 1=engaged */
    int   turbo_enabled; /* 0 or 1 */
    int   preset_id;
    enum ShiftPhase shift_phase;

#ifdef __cplusplus
    EngineParams()
        : rpm(800.0f), throttle(0.0f), gear(0), clutch(1.0f),
          turbo_enabled(0), preset_id(0), shift_phase(ShiftPhase_Idle) {}
#endif
};
