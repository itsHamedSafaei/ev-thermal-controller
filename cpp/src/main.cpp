#include "thermal_controller.hpp"

#include <iomanip>
#include <iostream>

int main() {
    SensorData data;
    data.battery_temperature_c = 42.0;
    data.temperature_sensor_valid = true;
    data.can_message_received = true;
    data.message_age_ms = 100;

    const double target_temperature_c = 30.0;
    const double proportional_gain = 5.0;

    const double temperature_error =
        data.battery_temperature_c - target_temperature_c;

    const double cooling_command_percent =
        calculate_cooling_command(
            data.battery_temperature_c,
            target_temperature_c,
            proportional_gain
        );

    const VehicleMode mode = determine_vehicle_mode(data);

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "Battery temperature: "
              << data.battery_temperature_c << " C\n";
    std::cout << "Target temperature: "
              << target_temperature_c << " C\n";
    std::cout << "Temperature error: "
              << temperature_error << " C\n";
    std::cout << "Cooling command: "
              << cooling_command_percent << "%\n";
    std::cout << "Vehicle mode: "
              << to_string(mode) << "\n";

    return 0;
}