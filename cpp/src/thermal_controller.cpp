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