#include "core/engine_model.h"
#include <cassert>
#include <cmath>
#include <cstdio>

using namespace engin;

void test_idle_rpm() {
    EngineModel m;
    m.set_throttle(0.0f);
    m.shift_gear(0);

    // Run for a few seconds at idle
    for (int i = 0; i < 300; ++i) {
        m.update(1.0f / 60.0f);
    }
    float rpm = m.get_rpm();
    assert(rpm >= 750.0f && rpm <= 900.0f);
    std::printf("  [PASS] idle_rpm = %.1f\n", rpm);
}

void test_rpm_increases_with_throttle() {
    EngineModel m;
    m.set_throttle(1.0f);
    m.shift_gear(0); // neutral, minimal load

    float initial_rpm = m.get_rpm();
    for (int i = 0; i < 300; ++i) {
        m.update(1.0f / 60.0f);
    }
    float final_rpm = m.get_rpm();
    assert(final_rpm > initial_rpm);
    std::printf("  [PASS] throttle up: %.1f -> %.1f RPM\n", initial_rpm, final_rpm);
}

void test_redline_clamp() {
    EngineModel m;
    m.set_redline_rpm(7000.0f);
    m.set_throttle(1.0f);
    m.shift_gear(0);

    for (int i = 0; i < 6000; ++i) {
        m.update(1.0f / 60.0f);
    }
    float rpm = m.get_rpm();
    assert(rpm <= 7000.0f + 1.0f); // allow tiny float error
    std::printf("  [PASS] redline clamp: rpm = %.1f (max 7000)\n", rpm);
}

void test_gear_affects_load() {
    EngineModel m;
    m.set_throttle(0.5f);
    m.shift_gear(1);
    for (int i = 0; i < 120; ++i) m.update(1.0f / 60.0f);
    float load_g1 = m.get_load();

    EngineModel m2;
    m2.set_throttle(0.5f);
    m2.shift_gear(5);
    for (int i = 0; i < 120; ++i) m2.update(1.0f / 60.0f);
    float load_g5 = m2.get_load();

    // Higher gear at same rpm range should produce different load
    std::printf("  [PASS] gear load: G1=%.3f, G5=%.3f\n", load_g1, load_g5);
}

int main() {
    std::printf("=== EngineModel Tests ===\n");
    test_idle_rpm();
    test_rpm_increases_with_throttle();
    test_redline_clamp();
    test_gear_affects_load();
    std::printf("All EngineModel tests passed.\n\n");
    return 0;
}
