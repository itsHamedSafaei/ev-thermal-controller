#ifndef THERMAL_CONTROLLER_HPP
#define THERMAL_CONTROLLER_HPP

double calculate_cooling_command(
    double battery_temperature_c,
    double target_temperature_c,
    double proportional_gain
);

#endif