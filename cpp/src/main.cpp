#include "thermal_controller.hpp"

#include <iomanip>
#include <iostream>

int main() {
    const double battery_temperature_c = 42.0;
    const double target_temperature_c = 30.0;
    const double proportional_gain = 5.0;

    const double temperature_error =
        battery_temperature_c - target_temperature_c;

    const double cooling_command_percent =
        calculate_cooling_command(
            battery_temperature_c,
            target_temperature_c,
            proportional_gain
        );

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "Battery temperature: "
              << battery_temperature_c << " C\n";
    std::cout << "Target temperature: "
              << target_temperature_c << " C\n";
    std::cout << "Temperature error: "
              << temperature_error << " C\n";
    std::cout << "Cooling command: "
              << cooling_command_percent << "%\n";

    return 0;
}