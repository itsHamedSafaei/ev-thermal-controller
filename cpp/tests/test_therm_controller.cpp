#include "thermal_controller.hpp"

#include <cmath>
#include <iostream>
#include <string>

static bool approximately_equal(
    double first,
    double second
) {
    constexpr double k_tolerance = 0.001;

    return std::fabs(first - second) < k_tolerance;
}

static void print_output(
    const ControllerOutput& output
) {
    std::cout
        << "mode=" << to_string(output.mode)
        << ", cooling=" << output.cooling_command_percent
        << ", charging_enabled=" << std::boolalpha
        << output.charging_enabled
        << ", charging_derated=" << output.charging_derated
        << ", fault_active=" << output.fault_active
        << ", fault_code=" << to_string(output.fault_code);
}

static bool run_test(
    const std::string& name,
    const SensorData& data,
    const ControllerOutput& expected
) {
    const ControllerOutput actual = update_controller(data);

    const bool passed =
        actual.mode == expected.mode &&
        approximately_equal(
            actual.cooling_command_percent,
            expected.cooling_command_percent
        ) &&
        actual.charging_enabled == expected.charging_enabled &&
        actual.charging_derated == expected.charging_derated &&
        actual.fault_active == expected.fault_active &&
        actual.fault_code == expected.fault_code;

    if (passed) {
        std::cout << "[PASS] " << name << '\n';
        return true;
    }

    std::cout << "[FAIL] " << name << '\n';

    std::cout << "  Expected: ";
    print_output(expected);
    std::cout << '\n';

    std::cout << "  Actual:   ";
    print_output(actual);
    std::cout << '\n';

    return false;
}

static SensorData make_valid_data() {
    SensorData data{};

    data.battery_temperature_c = 42.0;
    data.ambient_temperature_c = 25.0;
    data.charge_current_a = 100.0;
    data.temperature_sensor_valid = true;
    data.can_message_received = true;
    data.message_age_ms = 100;

    return data;
}

int main() {
    int passed = 0;
    int total = 0;

    {
        SensorData data = make_valid_data();
        data.temperature_sensor_valid = false;

        const ControllerOutput expected{
            VehicleMode::Fault,
            100.0,
            false,
            false,
            true,
            FaultCode::InvalidTemperatureSensor
        };

        ++total;
        if (run_test(
                "Invalid sensor enters safe fault state",
                data,
                expected
            )) {
            ++passed;
        }
    }

    {
        SensorData data = make_valid_data();
        data.can_message_received = false;

        const ControllerOutput expected{
            VehicleMode::Fault,
            100.0,
            false,
            false,
            true,
            FaultCode::MissingCanMessage
        };

        ++total;
        if (run_test(
                "Missing CAN message enters safe fault state",
                data,
                expected
            )) {
            ++passed;
        }
    }

    {
        SensorData data = make_valid_data();
        data.message_age_ms = 501;

        const ControllerOutput expected{
            VehicleMode::Fault,
            100.0,
            false,
            false,
            true,
            FaultCode::CanMessageTimeout
        };

        ++total;
        if (run_test(
                "CAN timeout enters safe fault state",
                data,
                expected
            )) {
            ++passed;
        }
    }

        {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 200.0;

        const ControllerOutput expected{
            VehicleMode::Fault,
            100.0,
            false,
            false,
            true,
            FaultCode::TemperatureOutOfRange
        };

        ++total;
        if (run_test(
                "Out-of-range temperature enters safe fault state",
                data,
                expected
            )) {
            ++passed;
        }
    }

    {
        // Verify the lower boundary of the valid simulated temperature range.
        //
        // Temperatures below -40.0C must be treated as invalid input data.
        SensorData data = make_valid_data();
        data.battery_temperature_c = -41.0;

        const ControllerOutput expected{
            VehicleMode::Fault,
            100.0,
            false,
            false,
            true,
            FaultCode::TemperatureOutOfRange
        };

        ++total;
        if (run_test(
                "-41.0C lower-range temperature fault",
                data,
                expected
            )) {
            ++passed;
        }
    }

    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 25.0;

        const ControllerOutput expected{
            VehicleMode::Normal,
            0.0,
            true,
            false,
            false,
            FaultCode::None
        };

        ++total;
        if (run_test(
                "25.0C normal operation",
                data,
                expected
            )) {
            ++passed;
        }
    }

    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 30.0;

        const ControllerOutput expected{
            VehicleMode::Normal,
            0.0,
            true,
            false,
            false,
            FaultCode::None
        };

        ++total;
        if (run_test(
                "30.0C remains normal",
                data,
                expected
            )) {
            ++passed;
        }
    }

    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 30.1;

        const ControllerOutput expected{
            VehicleMode::Cooling,
            0.5,
            true,
            false,
            false,
            FaultCode::None
        };

        ++total;
        if (run_test(
                "30.1C starts cooling at 0.5 percent",
                data,
                expected
            )) {
            ++passed;
        }
    }

    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 42.0;

        const ControllerOutput expected{
            VehicleMode::Cooling,
            60.0,
            true,
            false,
            false,
            FaultCode::None
        };

        ++total;
        if (run_test(
                "42.0C commands 60 percent cooling",
                data,
                expected
            )) {
            ++passed;
        }
    }

    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 49.9;

        const ControllerOutput expected{
            VehicleMode::Cooling,
            99.5,
            true,
            false,
            false,
            FaultCode::None
        };

        ++total;
        if (run_test(
                "49.9C remains cooling below derate threshold",
                data,
                expected
            )) {
            ++passed;
        }
    }

    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 50.0;

        const ControllerOutput expected{
            VehicleMode::DerateCharging,
            100.0,
            true,
            true,
            false,
            FaultCode::None
        };

        ++total;
        if (run_test(
                "50.0C starts charging derate",
                data,
                expected
            )) {
            ++passed;
        }
    }

    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 59.9;

        const ControllerOutput expected{
            VehicleMode::DerateCharging,
            100.0,
            true,
            true,
            false,
            FaultCode::None
        };

        ++total;
        if (run_test(
                "59.9C remains in charging derate",
                data,
                expected
            )) {
            ++passed;
        }
    }

    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 60.0;

        const ControllerOutput expected{
            VehicleMode::Fault,
            100.0,
            false,
            false,
            true,
            FaultCode::CriticalOvertemperature
        };

        ++total;
        if (run_test(
                "60.0C enters critical overtemperature fault",
                data,
                expected
            )) {
            ++passed;
        }
    }

    {
        SensorData data = make_valid_data();
        data.battery_temperature_c = 65.0;
        data.temperature_sensor_valid = false;

        const ControllerOutput expected{
            VehicleMode::Fault,
            100.0,
            false,
            false,
            true,
            FaultCode::InvalidTemperatureSensor
        };

        ++total;
        if (run_test(
                "Invalid sensor has priority over temperature reading",
                data,
                expected
            )) {
            ++passed;
        }
    }

    std::cout
        << "Summary: "
        << passed
        << "/"
        << total
        << " tests passed.\n";

    return (passed == total) ? 0 : 1;
}