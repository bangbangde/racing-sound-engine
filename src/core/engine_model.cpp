#include "core/engine_model.h"
#include <algorithm>
#include <cmath>

namespace engin {

EngineModel::EngineModel() = default;

void EngineModel::set_throttle(float value) {
    throttle_ = std::clamp(value, 0.0f, 1.0f);
}

void EngineModel::shift_gear(int gear) {
    gear_ = std::clamp(gear, 0, 6);
}

void EngineModel::set_clutch(float value) {
    clutch_ = std::clamp(value, 0.0f, 1.0f);
}

void EngineModel::update(float dt) {
    // Compute load from gear ratio — higher gear = more load at same RPM
    float gear_ratio = gear_ratios_[gear_];
    if (gear_ == 0 || clutch_ < 0.1f) {
        // Neutral or clutch disengaged: no drivetrain load
        load_ = 0.05f; // minimal accessories load
    } else {
        // Load proportional to gear ratio and RPM fraction
        float rpm_frac = current_rpm_ / redline_rpm_;
        load_ = 0.1f + 0.6f * gear_ratio / 3.5f * rpm_frac * clutch_;
    }

    // Torque model: throttle drives, friction and load resist
    float drive_torque = throttle_ * max_torque_;
    float resist_torque = friction_ + load_ * max_torque_ * 0.5f;

    // RPM derivative
    float rpm_dot = (drive_torque - resist_torque) / inertia_;

    current_rpm_ += rpm_dot * dt;

    // Clamp RPM
    if (current_rpm_ < idle_rpm_) {
        // Idle governor: pull RPM back up
        current_rpm_ = idle_rpm_ + (current_rpm_ - idle_rpm_) * 0.9f;
        if (current_rpm_ < idle_rpm_) current_rpm_ = idle_rpm_;
    }
    if (current_rpm_ > redline_rpm_) {
        // Rev limiter: hard cut
        current_rpm_ = redline_rpm_;
    }
}

} // namespace engin
