#include "thermal_controller.hpp"

#include <algorithm>

namespace {

constexpr double k_target_temperature_c = 30.0;
constexpr double k_derate_temperature_c = 50.0;
constexpr double k_critical_temperature_c = 60.0;
constexpr double k_min_valid_temperature_c = -40.0;
constexpr double k_max_valid_temperature_c = 120.0;
constexpr int k_message_timeout_ms = 500;
constexpr double k_proportional_gain = 5.0;

ControllerOutput make_fault_output(FaultCode code) {
    return {
        VehicleMode::Fault,
        100.0,
        false,
        false,
        true,
        code
    };
}

}  // namespace

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

bool is_temperature_reading_valid(
    const SensorData& data
) {
    if (!data.temperature_sensor_valid) {
        return false;
    }

    if (!data.can_message_received) {
        return false;
    }

    if (data.message_age_ms > k_message_timeout_ms) {
        return false;
    }

    if (data.battery_temperature_c < k_min_valid_temperature_c ||
        data.battery_temperature_c > k_max_valid_temperature_c) {
        return false;
    }

    return true;
}

ControllerOutput update_controller(
    const SensorData& data
) {
    if (!data.temperature_sensor_valid) {
        return make_fault_output(FaultCode::InvalidTemperatureSensor);
    }

    if (!data.can_message_received) {
        return make_fault_output(FaultCode::MissingCanMessage);
    }

    if (data.message_age_ms > k_message_timeout_ms) {
        return make_fault_output(FaultCode::CanMessageTimeout);
    }

    if (data.battery_temperature_c < k_min_valid_temperature_c ||
        data.battery_temperature_c > k_max_valid_temperature_c) {
        return make_fault_output(FaultCode::TemperatureOutOfRange);
    }

    if (data.battery_temperature_c >= k_critical_temperature_c) {
        return make_fault_output(FaultCode::CriticalOvertemperature);
    }

    const double cooling_command_percent =
        calculate_cooling_command(
            data.battery_temperature_c,
            k_target_temperature_c,
            k_proportional_gain
        );

    if (data.battery_temperature_c >= k_derate_temperature_c) {
        return {
            VehicleMode::DerateCharging,
            100.0,
            true,
            true,
            false,
            FaultCode::None
        };
    }

    if (data.battery_temperature_c > k_target_temperature_c) {
        return {
            VehicleMode::Cooling,
            cooling_command_percent,
            true,
            false,
            false,
            FaultCode::None
        };
    }

    return {
        VehicleMode::Normal,
        cooling_command_percent,
        true,
        false,
        false,
        FaultCode::None
    };
}

VehicleMode determine_vehicle_mode(
    const SensorData& data
) {
    return update_controller(data).mode;
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

    return "UNKNOWN_MODE";
}

const char* to_string(FaultCode code) {
    switch (code) {
        case FaultCode::None:
            return "NONE";

        case FaultCode::InvalidTemperatureSensor:
            return "INVALID_TEMPERATURE_SENSOR";

        case FaultCode::MissingCanMessage:
            return "MISSING_CAN_MESSAGE";

        case FaultCode::CanMessageTimeout:
            return "CAN_MESSAGE_TIMEOUT";

        case FaultCode::TemperatureOutOfRange:
            return "TEMPERATURE_OUT_OF_RANGE";

        case FaultCode::CriticalOvertemperature:
            return "CRITICAL_OVERTEMPERATURE";
    }

    return "UNKNOWN_FAULT";
}