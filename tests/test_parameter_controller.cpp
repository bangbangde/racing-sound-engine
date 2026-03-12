#include "core/parameter_controller.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <thread>
#include <atomic>

using namespace engin;

void test_initial_state() {
    ParameterController ctrl;
    const EngineParams& p = ctrl.read();
    assert(p.rpm == 800.0f);
    assert(p.throttle == 0.0f);
    assert(p.gear == 0);
    std::printf("  [PASS] initial state correct\n");
}

void test_write_read() {
    ParameterController ctrl;

    EngineParams params;
    params.rpm      = 5000.0f;
    params.throttle = 0.75f;
    params.gear     = 3;
    ctrl.write(params);

    const EngineParams& p = ctrl.read();
    assert(p.rpm == 5000.0f);
    assert(p.throttle == 0.75f);
    assert(p.gear == 3);
    std::printf("  [PASS] write/read roundtrip\n");
}

void test_concurrent_access() {
    ParameterController ctrl;
    std::atomic<bool> running{true};
    std::atomic<int>  read_count{0};
    std::atomic<int>  error_count{0};

    // Writer thread: rapidly update params
    std::thread writer([&]() {
        float rpm = 800.0f;
        while (running.load()) {
            EngineParams p;
            p.rpm      = rpm;
            p.throttle = rpm / 8000.0f;
            p.gear     = static_cast<int>(rpm / 1500.0f) + 1;
            ctrl.write(p);
            rpm += 1.0f;
            if (rpm > 8000.0f) rpm = 800.0f;
        }
    });

    // Reader thread: rapidly read and verify consistency
    std::thread reader([&]() {
        while (running.load()) {
            const EngineParams& p = ctrl.read();
            // Verify data consistency: rpm should be in valid range
            if (p.rpm < 799.0f || p.rpm > 8001.0f) {
                error_count.fetch_add(1);
            }
            // Verify throttle is consistent with rpm
            float expected_throttle = p.rpm / 8000.0f;
            if (std::fabs(p.throttle - expected_throttle) > 0.01f) {
                // This would indicate a torn read (reading partial update)
                error_count.fetch_add(1);
            }
            read_count.fetch_add(1);
        }
    });

    // Run for 100ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    running.store(false);

    writer.join();
    reader.join();

    int reads = read_count.load();
    int errors = error_count.load();
    assert(errors == 0);
    assert(reads > 100); // Should have done many reads
    std::printf("  [PASS] concurrent access: %d reads, %d errors\n", reads, errors);
}

int main() {
    std::printf("=== ParameterController Tests ===\n");
    test_initial_state();
    test_write_read();
    test_concurrent_access();
    std::printf("All ParameterController tests passed.\n\n");
    return 0;
}
