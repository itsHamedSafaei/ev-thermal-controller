// This file runs simulated EV battery thermal-control scenarios.
//
// It prints readable output for a person in the terminal and writes the
// same results to a CSV file for future Python-based test automation.
//
// This is an educational simulation only. It does not communicate with a
// real vehicle, battery, charger, ECU, or CAN network.

#include "thermal_controller.hpp"

// <array> stores a fixed list of scenarios.
// <filesystem> creates the output data directory when needed.
// <fstream> writes CSV output to a file.
// <iomanip> formats decimal values, such as 60.0 instead of 60.
// <iostream> provides terminal output with std::cout and std::cerr.
#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <system_error>

// A Scenario combines a readable name with simulated controller input data.
struct Scenario {
    const char* name;
    SensorData data;
};

// Creates standard simulated sensor/CAN input data.
//
// Most scenarios use valid input values by default. A specific scenario can
// override only the value it needs, such as sensor validity or message age.
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

// Converts a Boolean value into readable CSV text.
static const char* bool_to_string(
    bool value
) {
    return value ? "true" : "false";
}

// Writes the first row of the CSV file.
//
// The header names explain the meaning of every value in later CSV rows.
static void write_csv_header(
    std::ofstream& csv_file
) {
    csv_file
        << "scenario,"
        << "battery_temperature_c,"
        << "ambient_temperature_c,"
        << "charge_current_a,"
        << "temperature_sensor_valid,"
        << "can_message_received,"
        << "message_age_ms,"
        << "mode,"
        << "cooling_command_percent,"
        << "charging_enabled,"
        << "charging_derated,"
        << "fault_active,"
        << "fault_code\n";
}

// Writes one completed controller scenario to one CSV row.
//
// Scenario names are controlled by this program and do not contain commas,
// so they can safely be written directly into this simple CSV format.
static void write_csv_result(
    std::ofstream& csv_file,
    const Scenario& scenario,
    const ControllerOutput& output
) {
    csv_file << std::fixed << std::setprecision(1);

    csv_file
        << scenario.name << ','
        << scenario.data.battery_temperature_c << ','
        << scenario.data.ambient_temperature_c << ','
        << scenario.data.charge_current_a << ','
        << bool_to_string(scenario.data.temperature_sensor_valid) << ','
        << bool_to_string(scenario.data.can_message_received) << ','
        << scenario.data.message_age_ms << ','
        << to_string(output.mode) << ','
        << output.cooling_command_percent << ','
        << bool_to_string(output.charging_enabled) << ','
        << bool_to_string(output.charging_derated) << ','
        << bool_to_string(output.fault_active) << ','
        << to_string(output.fault_code)
        << '\n';
}

// Runs the controller once for one simulated scenario, prints the input and
// result to the terminal, then returns the output for CSV logging.
static ControllerOutput print_scenario(
    const Scenario& scenario
) {
    // The controller receives SensorData and returns the complete decision:
    // operating mode, cooling command, charging state, and fault information.
    const ControllerOutput output =
        update_controller(scenario.data);

    // Print the simulated input values used by this scenario.
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

    // Print the output decision calculated by the controller.
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

    return output;
}

int main() {
    // Define the simulated conditions used to demonstrate normal behavior,
    // charge derating, sensor faults, communication faults, and temperature
    // safety faults.
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

    // Create the data directory if it does not already exist.
    //
    // The demo runs from cpp/, so ../data points to the repository's data/
    // directory: ev-thermal-controller/data/.
    const std::filesystem::path data_directory = "../data";
    std::error_code filesystem_error;

    std::filesystem::create_directories(
        data_directory,
        filesystem_error
    );

    if (filesystem_error) {
        std::cerr
            << "Error: could not create data directory: "
            << filesystem_error.message()
            << '\n';

        return 1;
    }

    // Open the CSV file for writing. Each demo run replaces the old generated
    // result file with fresh output from the current controller behavior.
    const std::filesystem::path csv_path =
        data_directory / "scenario_results.csv";

    std::ofstream csv_file(csv_path);

    if (!csv_file.is_open()) {
        std::cerr
            << "Error: could not open CSV output file: "
            << csv_path
            << '\n';

        return 1;
    }

    // Write the CSV column names before writing individual scenario results.
    write_csv_header(csv_file);

    // Format terminal decimal values with one decimal place and print Boolean
    // values as true/false rather than 1/0.
    std::cout << std::fixed << std::setprecision(1);
    std::cout << std::boolalpha;

    // Print a title so the terminal output is easy to identify.
    std::cout << "EV Battery Thermal Controller Scenario Runner\n";
    std::cout << "================================================\n\n";

    // Run every scenario, print the result to the terminal, and save the same
    // controller decision into the CSV file.
    for (const Scenario& scenario : scenarios) {
        const ControllerOutput output =
            print_scenario(scenario);

        write_csv_result(
            csv_file,
            scenario,
            output
        );
    }

    // Close the CSV file after all scenario results have been written.
    csv_file.close();

    // Check whether writing the file succeeded before reporting completion.
    if (!csv_file) {
        std::cerr
            << "Error: failed while writing CSV results.\n";

        return 1;
    }

    std::cout
        << "CSV results written to: "
        << csv_path
        << '\n';

    // Return zero to indicate that the demo completed successfully.
    return 0;
}