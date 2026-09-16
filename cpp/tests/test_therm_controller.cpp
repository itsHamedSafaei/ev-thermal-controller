#include "thermal_controller.hpp"
#include <iostream>
#include <string>

static bool run_test(const std::string& name, const SensorData& data, VehicleMode expected) {
    VehicleMode result = determine_vehicle_mode(data);
    if (result == expected) {
        std::cout << "[PASS] " << name << std::endl;
        return true;
    }

    std::cout << "[FAIL] " << name
              << " - got " << to_string(result)
              << ", expected " << to_string(expected) << std::endl;
    return false;
}

static SensorData make_valid_data() {
    SensorData d{};
    d.battery_temperature_c = 42.0;
    d.temperature_sensor_valid = true;
    d.can_message_received = true;
    d.message_age_ms = 100;
    return d;
}

int main() {
    int passed = 0;
    int total = 0;

    // Fault tests (existing)
    {
        SensorData data = make_valid_data();
        data.temperature_sensor_valid = false;
        ++total;
        if (run_test("Invalid sensor -> Fault", data, VehicleMode::Fault)) ++passed;
    }

    {
        SensorData data = make_valid_data();
        data.can_message_received = false;
        ++total;
        if (run_test("Missing CAN message -> Fault", data, VehicleMode::Fault)) ++passed;
    }

    {
        SensorData data = make_valid_data();
        data.message_age_ms = 1000; // > 500
        ++total;
        if (run_test("Stale message -> Fault", data, VehicleMode::Fault)) ++passed;
    }

    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 200.0;
        ++total;
        if (run_test("Impossible temperature -> Fault", data, VehicleMode::Fault)) ++passed;
    }

    // Normal path
    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 25.0;
        ++total;
        if (run_test("Normal operation -> Normal", data, VehicleMode::Normal)) ++passed;
    }

    // Boundary tests for normal operating temperatures
    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 30.0;
        ++total;
        if (run_test("Boundary 30.0C -> NORMAL", data, VehicleMode::Normal)) ++passed;
    }

    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 30.1;
        ++total;
        if (run_test("Boundary 30.1C -> COOLING", data, VehicleMode::Cooling)) ++passed;
    }

    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 49.9;
        ++total;
        if (run_test("Boundary 49.9C -> COOLING", data, VehicleMode::Cooling)) ++passed;
    }

    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 50.0;
        ++total;
        if (run_test("Boundary 50.0C -> DERATE_CHARGING", data, VehicleMode::DerateCharging)) ++passed;
    }

    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 59.9;
        ++total;
        if (run_test("Boundary 59.9C -> DERATE_CHARGING", data, VehicleMode::DerateCharging)) ++passed;
    }

    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 60.0;
        ++total;
        if (run_test("Boundary 60.0C -> FAULT", data, VehicleMode::Fault)) ++passed;
    }

    std::cout << "Summary: " << passed << "/" << total << " tests passed." << std::endl;

    return (passed == total) ? 0 : 1;
}