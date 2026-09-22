#ifndef THERMAL_CONTROLLER_HPP
#define THERMAL_CONTROLLER_HPP

enum class VehicleMode {
    Normal,
    Cooling,
    DerateCharging,
    Fault
};

enum class FaultCode {
    None,
    InvalidTemperatureSensor,
    MissingCanMessage,
    CanMessageTimeout,
    TemperatureOutOfRange,
    CriticalOvertemperature
};

struct SensorData {
    double battery_temperature_c;
    double ambient_temperature_c;
    double charge_current_a;
    bool temperature_sensor_valid;
    bool can_message_received;
    int message_age_ms;
};

struct ControllerOutput {
    VehicleMode mode;
    double cooling_command_percent;
    bool charging_enabled;
    bool charging_derated;
    bool fault_active;
    FaultCode fault_code;
};

double calculate_cooling_command(
    double battery_temperature_c,
    double target_temperature_c,
    double proportional_gain
);

bool is_temperature_reading_valid(
    const SensorData& data
);

VehicleMode determine_vehicle_mode(
    const SensorData& data
);

ControllerOutput update_controller(
    const SensorData& data
);

const char* to_string(VehicleMode mode);
const char* to_string(FaultCode code);

#endif