#ifndef THERMAL_CONTROLLER_HPP
#define THERMAL_CONTROLLER_HPP

enum class VehicleMode {
    Normal,
    Cooling,
    DerateCharging,
    Fault
};

double calculate_cooling_command(
    double battery_temperature_c,
    double target_temperature_c,
    double proportional_gain
);

VehicleMode determine_vehicle_mode(
    double battery_temperature_c
);

const char* to_string(VehicleMode mode);

#endif