// This file runs simulated EV battery thermal-control scenarios.
//
// It prints readable results to the terminal and writes CSV output for
// Python-based validation.
//
// This file includes:
// - Individual controller scenarios
// - A simulated periodic controller sequence at 100 ms intervals
// - CSV logging for both result types
//
// This is an educational simulation only. It does not communicate with a
// real vehicle, battery, charger, ECU, or CAN network.

#include "thermal_controller.hpp"

// <array> stores fixed collections of scenarios and periodic inputs.
// <filesystem> creates the output data directory when needed.
// <fstream> writes CSV output files.
// <iomanip> formats decimal values, such as 60.0 instead of 60.
// <iostream> provides terminal output with std::cout and std::cerr.
#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <system_error>

// A Scenario combines a readable name with one set of simulated input data.
struct Scenario {
    const char* name;
    SensorData data;
};

// A PeriodicStep combines a simulated timestamp with one set of controller
// input data. Each step represents one controller update in the 100 ms loop.
struct PeriodicStep {
    int time_ms;
    SensorData data;
};

// Creates standard simulated sensor/CAN input data.
//
// Most inputs use valid defaults. Individual scenarios or periodic steps can
// override the sensor validity, CAN-message state, or message age as needed.
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

// Writes the common input and output fields used by both CSV files.
//
// The scenario CSV writes a scenario name first.
// The periodic CSV writes a simulated time value first.
static void write_csv_input_and_output_fields(
    std::ofstream& csv_file,
    const SensorData& data,
    const ControllerOutput& output
) {
    csv_file << std::fixed << std::setprecision(1);

    csv_file
        << data.battery_temperature_c << ','
        << data.ambient_temperature_c << ','
        << data.charge_current_a << ','
        << bool_to_string(data.temperature_sensor_valid) << ','
        << bool_to_string(data.can_message_received) << ','
        << data.message_age_ms << ','
        << to_string(output.mode) << ','
        << output.cooling_command_percent << ','
        << bool_to_string(output.charging_enabled) << ','
        << bool_to_string(output.charging_derated) << ','
        << bool_to_string(output.fault_active) << ','
        << to_string(output.fault_code);
}

// Writes the first row of the scenario-result CSV file.
static void write_scenario_csv_header(
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

// Writes one scenario result to the scenario CSV file.
static void write_scenario_csv_result(
    std::ofstream& csv_file,
    const Scenario& scenario,
    const ControllerOutput& output
) {
    // Scenario names are controlled by this program and contain no commas.
    csv_file << scenario.name << ',';

    write_csv_input_and_output_fields(
        csv_file,
        scenario.data,
        output
    );

    csv_file << '\n';
}

// Writes the first row of the periodic-result CSV file.
static void write_periodic_csv_header(
    std::ofstream& csv_file
) {
    csv_file
        << "time_ms,"
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

// Writes one periodic controller update to the periodic CSV file.
static void write_periodic_csv_result(
    std::ofstream& csv_file,
    const PeriodicStep& step,
    const ControllerOutput& output
) {
    csv_file << step.time_ms << ',';

    write_csv_input_and_output_fields(
        csv_file,
        step.data,
        output
    );

    csv_file << '\n';
}

// Runs one named scenario, prints the input/output result, and returns the
// output so the caller can save it to the scenario CSV file.
static ControllerOutput print_scenario(
    const Scenario& scenario
) {
    const ControllerOutput output =
        update_controller(scenario.data);

    // Print simulated scenario inputs.
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

    // Print the controller decision calculated from those inputs.
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

// Runs one simulated periodic controller update and prints a shorter summary.
//
// The time value is simulated. No real delay or sleep call is used.
static ControllerOutput print_periodic_step(
    const PeriodicStep& step
) {
    const ControllerOutput output =
        update_controller(step.data);

    std::cout << "Time: "
              << step.time_ms
              << " ms\n";

    std::cout << "  Battery temperature: "
              << step.data.battery_temperature_c
              << " C\n";

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
    // Define independent scenarios for normal, cooling, derate, and faults.
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

    // Define a simulated periodic sequence.
    //
    // Each element represents one controller update every 100 ms. The values
    // are processed immediately rather than waiting in real time.
    const std::array<PeriodicStep, 7> periodic_steps{{
        {
            0,
            make_sensor_data(25.0)
        },
        {
            100,
            make_sensor_data(32.0)
        },
        {
            200,
            make_sensor_data(42.0)
        },
        {
            300,
            make_sensor_data(55.0)
        },
        {
            400,
            make_sensor_data(60.0)
        },
        {
            500,
            make_sensor_data(42.0)
        },
        {
            600,
            make_sensor_data(25.0)
        }
    }};

    // Create the generated-data directory if it does not exist.
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

    // Define paths for the two generated CSV files.
    const std::filesystem::path scenario_csv_path =
        data_directory / "scenario_results.csv";

    const std::filesystem::path periodic_csv_path =
        data_directory / "periodic_results.csv";

    // Open the scenario CSV file for fresh output.
    std::ofstream scenario_csv_file(scenario_csv_path);

    if (!scenario_csv_file.is_open()) {
        std::cerr
            << "Error: could not open scenario CSV output file: "
            << scenario_csv_path
            << '\n';

        return 1;
    }

    // Open the periodic CSV file for fresh output.
    std::ofstream periodic_csv_file(periodic_csv_path);

    if (!periodic_csv_file.is_open()) {
        std::cerr
            << "Error: could not open periodic CSV output file: "
            << periodic_csv_path
            << '\n';

        return 1;
    }

    // Write CSV column headers before writing result rows.
    write_scenario_csv_header(scenario_csv_file);
    write_periodic_csv_header(periodic_csv_file);

    // Format terminal decimal values with one decimal place and print Boolean
    // values as true/false instead of 1/0.
    std::cout << std::fixed << std::setprecision(1);
    std::cout << std::boolalpha;

    // Print the scenario-runner title.
    std::cout << "EV Battery Thermal Controller Scenario Runner\n";
    std::cout << "================================================\n\n";

    // Run and log every independent scenario.
    for (const Scenario& scenario : scenarios) {
        const ControllerOutput output =
            print_scenario(scenario);

        write_scenario_csv_result(
            scenario_csv_file,
            scenario,
            output
        );
    }

    // Run and log the simulated periodic 100 ms sequence.
    std::cout << "Simulated Periodic Controller Sequence\n";
    std::cout << "======================================\n";
    std::cout << "Update interval: 100 ms\n\n";

    for (const PeriodicStep& step : periodic_steps) {
        const ControllerOutput output =
            print_periodic_step(step);

        write_periodic_csv_result(
            periodic_csv_file,
            step,
            output
        );
    }

    // Close both generated CSV files after all results are written.
    scenario_csv_file.close();
    periodic_csv_file.close();

    // Check whether the scenario CSV write completed successfully.
    if (!scenario_csv_file) {
        std::cerr
            << "Error: failed while writing scenario CSV results.\n";

        return 1;
    }

    // Check whether the periodic CSV write completed successfully.
    if (!periodic_csv_file) {
        std::cerr
            << "Error: failed while writing periodic CSV results.\n";

        return 1;
    }

    // Report generated CSV locations.
    std::cout
        << "Scenario CSV results written to: "
        << scenario_csv_path
        << '\n';

    std::cout
        << "Periodic CSV results written to: "
        << periodic_csv_path
        << '\n';

    // Return zero to indicate successful completion.
    return 0;
}