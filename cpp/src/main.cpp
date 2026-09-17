#include "thermal_controller.hpp"

#include <iomanip>
#include <iostream>

int main() {
    SensorData data{};

    data.battery_temperature_c = 42.0;
    data.ambient_temperature_c = 25.0;
    data.charge_current_a = 100.0;
    data.temperature_sensor_valid = true;
    data.can_message_received = true;
    data.message_age_ms = 100;

    const ControllerOutput output = update_controller(data);

    std::cout << std::fixed << std::setprecision(1);
    std::cout << std::boolalpha;

    std::cout << "EV Battery Thermal Controller Demo\n";
    std::cout << "==================================\n\n";

    std::cout << "Simulated inputs\n";
    std::cout << "Battery temperature: "
              << data.battery_temperature_c
              << " C\n";

    std::cout << "Ambient temperature: "
              << data.ambient_temperature_c
              << " C\n";

    std::cout << "Charge current: "
              << data.charge_current_a
              << " A\n";

    std::cout << "Temperature sensor valid: "
              << data.temperature_sensor_valid
              << '\n';

    std::cout << "CAN message received: "
              << data.can_message_received
              << '\n';

    std::cout << "CAN message age: "
              << data.message_age_ms
              << " ms\n\n";

    std::cout << "Controller output\n";
    std::cout << "Mode: "
              << to_string(output.mode)
              << '\n';

    std::cout << "Cooling command: "
              << output.cooling_command_percent
              << "%\n";

    std::cout << "Charging enabled: "
              << output.charging_enabled
              << '\n';

    std::cout << "Charging derated: "
              << output.charging_derated
              << '\n';

    std::cout << "Fault active: "
              << output.fault_active
              << '\n';

    std::cout << "Fault code: "
              << to_string(output.fault_code)
              << '\n';

    return 0;
}