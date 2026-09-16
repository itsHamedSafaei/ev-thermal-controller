#ifndef THERMAL_CONTROLLER_HPP
#define THERMAL_CONTROLLER_HPP

enum class VehicleMode {
    Normal,
    Cooling,
    DerateCharging,
    Fault
};

struct SensorData {
    double battery_temperature_c;
    bool temperature_sensor_valid;
    bool can_message_received;
    int message_age_ms;
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

const char* to_string(VehicleMode mode);

#endif