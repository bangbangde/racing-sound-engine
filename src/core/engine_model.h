#pragma once

#include "core/types.h"

namespace engin {

class EngineModel {
public:
    EngineModel();

    void set_throttle(float value);  // 0.0 ~ 1.0
    void shift_gear(int gear);       // 0=neutral, 1-6
    void set_clutch(float value);    // 0=disengaged, 1=engaged

    // Call at fixed rate (e.g. 60Hz) from control thread
    void update(float dt_seconds);

    float get_rpm() const { return current_rpm_; }
    float get_throttle() const { return throttle_; }
    float get_load() const { return load_; }
    int   get_gear() const { return gear_; }
    float get_clutch() const { return clutch_; }

    void set_idle_rpm(float rpm) { idle_rpm_ = rpm; }
    void set_redline_rpm(float rpm) { redline_rpm_ = rpm; }
    void set_inertia(float inertia) { inertia_ = inertia; }

private:
    float current_rpm_  = 800.0f;
    float throttle_     = 0.0f;
    float load_         = 0.0f;
    int   gear_         = 0;
    float clutch_       = 1.0f;

    float idle_rpm_     = 800.0f;
    float redline_rpm_  = 8000.0f;
    float inertia_      = 0.15f;    // moment of inertia factor
    float friction_     = 50.0f;    // friction torque
    float max_torque_   = 5000.0f;  // max engine torque

    // Gear ratios (gear 0 = neutral)
    static constexpr float gear_ratios_[7] = {
        0.0f, 3.5f, 2.5f, 1.8f, 1.4f, 1.1f, 0.9f
    };
};

} // namespace engin
