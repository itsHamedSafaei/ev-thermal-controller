#include "thermal_controller.hpp"

#include <algorithm>

double calculate_cooling_command(
    double battery_temperature_c,
    double target_temperature_c,
    double proportional_gain
) {
    const double temperature_error =
        battery_temperature_c - target_temperature_c;

    const double requested_cooling_percent =
        proportional_gain * temperature_error;

    return std::clamp(
        requested_cooling_percent,
        0.0,
        100.0
    );
}

VehicleMode determine_vehicle_mode(
    double battery_temperature_c
) {
    if (battery_temperature_c >= 60.0) {
        return VehicleMode::Fault;
    }

    if (battery_temperature_c >= 50.0) {
        return VehicleMode::DerateCharging;
    }

    if (battery_temperature_c > 30.0) {
        return VehicleMode::Cooling;
    }

    return VehicleMode::Normal;
}

const char* to_string(VehicleMode mode) {
    switch (mode) {
        case VehicleMode::Normal:
            return "NORMAL";
        case VehicleMode::Cooling:
            return "COOLING";
        case VehicleMode::DerateCharging:
            return "DERATE_CHARGING";
        case VehicleMode::Fault:
            return "FAULT";
    }

    return "UNKNOWN";
}