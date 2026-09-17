// This file runs simulated EV battery thermal-control scenarios.
//
// It is a demonstration program only. It does not communicate with a real
// vehicle, battery, charger, ECU, or CAN network.

#include "thermal_controller.hpp"

// <array> stores a fixed list of scenarios.
// <iomanip> formats decimal values, such as 60.0 instead of 60.
// <iostream> provides std::cout for terminal output.
#include <array>
#include <iomanip>
#include <iostream>

// A Scenario combines a readable name with simulated controller input data.
// This makes it easy to run several vehicle conditions using the same code.
struct Scenario {
    const char* name;
    SensorData data;
};

// Creates standard simulated sensor/CAN input data.
//
// Most scenarios have valid inputs by default. Individual scenarios can
// override only the value they need, such as making a sensor invalid or
// making the CAN message older than the timeout limit.
static SensorData make_sensor_data(
    double battery_temperature_c,
    bool temperature_sensor_valid = true,
    bool can_message_received = true,
    int message_age_ms = 100
) {
    return {
        battery_temperature_c,
        25.0,
        100.0,
        temperature_sensor_valid,
        can_message_received,
        message_age_ms
    };
}

// Runs the controller once for one simulated scenario and prints both the
// controller inputs and the resulting controller decision.
static void print_scenario(
    const Scenario& scenario
) {
    // The controller receives SensorData and returns the complete decision:
    // operating mode, cooling command, charging state, and fault information.
    const ControllerOutput output =
        update_controller(scenario.data);

    // Print the simulated inputs used by this scenario.
    std::cout << "Scenario: " << scenario.name << '\n';

    std::cout << "  Battery temperature: "
              << scenario.data.battery_temperature_c
              << " C\n";

    std::cout << "  Ambient temperature: "
              << scenario.data.ambient_temperature_c
              << " C\n";

    std::cout << "  Charge current: "
              << scenario.data.charge_current_a
              << " A\n";

    std::cout << "  Temperature sensor valid: "
              << scenario.data.temperature_sensor_valid
              << '\n';

    std::cout << "  CAN message received: "
              << scenario.data.can_message_received
              << '\n';

    std::cout << "  CAN message age: "
              << scenario.data.message_age_ms
              << " ms\n";

    // Print the controller outputs calculated from the simulated inputs.
    std::cout << "  Mode: "
              << to_string(output.mode)
              << '\n';

    std::cout << "  Cooling command: "
              << output.cooling_command_percent
              << "%\n";

    std::cout << "  Charging enabled: "
              << output.charging_enabled
              << '\n';

    std::cout << "  Charging derated: "
              << output.charging_derated
              << '\n';

    std::cout << "  Fault active: "
              << output.fault_active
              << '\n';

    std::cout << "  Fault code: "
              << to_string(output.fault_code)
              << "\n\n";
}

int main() {
    // Define the simulated scenarios that demonstrate the controller's
    // normal behavior, charging derate behavior, and safe fault responses.
    const std::array<Scenario, 7> scenarios{{
        {
            "Normal operation",
            make_sensor_data(25.0)
        },
        {
            "Cooling required",
            make_sensor_data(42.0)
        },
        {
            "Charging derate",
            make_sensor_data(55.0)
        },
        {
            "Invalid temperature sensor",
            make_sensor_data(42.0, false)
        },
        {
            "CAN message timeout",
            make_sensor_data(42.0, true, true, 501)
        },
        {
            "Out-of-range temperature",
            make_sensor_data(200.0)
        },
        {
            "Critical overtemperature",
            make_sensor_data(60.0)
        }
    }};

    // Format floating-point values with one decimal place and print Boolean
    // values as true/false instead of 1/0.
    std::cout << std::fixed << std::setprecision(1);
    std::cout << std::boolalpha;

    // Print a title so the terminal output is easy to identify.
    std::cout << "EV Battery Thermal Controller Scenario Runner\n";
    std::cout << "================================================\n\n";

    // Run the controller once for every scenario in the fixed scenario list.
    for (const Scenario& scenario : scenarios) {
        print_scenario(scenario);
    }

    // Return zero to indicate that the demo program completed successfully.
    return 0;
}