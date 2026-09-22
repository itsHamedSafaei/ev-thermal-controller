# EV Battery Thermal Controller and CAN Test Simulator

A C++20 educational simulation of an electric-vehicle battery thermal controller. The project models temperature-based cooling, charging derate, fault handling, simulated CAN-style input checks, periodic controller execution, CSV logging, C++ unit tests, and independent Python validation.

> **Simulation-only scope:** This project does not communicate with a real vehicle, battery, charger, ECU, or CAN network. It is intended for learning, portfolio development, and software-testing practice.

## Overview

The controller receives simulated battery and vehicle data, evaluates safety conditions, and returns a safe controller output.

The project demonstrates:

- Temperature-based battery cooling commands
- Vehicle operating modes: `NORMAL`, `COOLING`, `DERATE_CHARGING`, and `FAULT`
- Charging derate behavior at elevated battery temperatures
- Safe-state handling for sensor, temperature-range, and CAN-style communication faults
- A deterministic simulated periodic controller sequence with a 100 ms update interval
- CSV logging of controller inputs and outputs
- C++ unit testing and Python-based software-in-the-loop-style validation

## Controller behavior

The controller uses simplified educational thresholds. They are not production vehicle specifications.

| Condition | Controller behavior |
|---|---|
| Battery temperature at or below 30°C | `NORMAL`, 0% cooling, charging enabled |
| Above 30°C and below 50°C | `COOLING`, proportional cooling command from 0% to 100%, charging enabled |
| From 50°C to below 60°C | `DERATE_CHARGING`, 100% cooling, charging remains enabled but is derated |
| At or above 60°C | `FAULT`, 100% cooling, charging disabled, `CRITICAL_OVERTEMPERATURE` |
| Temperature sensor invalid | `FAULT`, 100% cooling, charging disabled |
| Temperature below -40°C or above 120°C | `FAULT`, `TEMPERATURE_OUT_OF_RANGE` |
| CAN-style message missing | `FAULT`, `CAN_MESSAGE_MISSING` |
| CAN-style message older than 500 ms | `FAULT`, `CAN_MESSAGE_TIMEOUT` |

The controller checks safety-related faults before normal operating behavior. For example, invalid temperature-sensor data takes priority over an otherwise valid temperature value.

## Periodic simulation

The demo includes a deterministic periodic controller sequence at simulated 100 ms intervals.

```text
0 ms    | 25.0°C | NORMAL
100 ms  | 32.0°C | COOLING
200 ms  | 42.0°C | COOLING
300 ms  | 55.0°C | DERATE_CHARGING
400 ms  | 60.0°C | FAULT
500 ms  | 42.0°C | COOLING
600 ms  | 25.0°C | NORMAL
```

The simulation processes these timestamps immediately; it does not wait in real time with `sleep()` calls. This keeps the result fast, deterministic, and easy to test.

The current controller is intentionally stateless: every call evaluates the current simulated inputs and returns the appropriate current output. Therefore, after the 60°C fault sample, the controller returns to `COOLING` at 42°C and `NORMAL` at 25°C. A stateful fault-latching and recovery policy would be a future enhancement.

## Architecture

```text
Simulated sensor and CAN-style inputs
                |
                v
      C++ thermal controller
                |
                v
Controller output:
mode, cooling command, charging state, fault state
                |
                v
C++ scenario runner and periodic simulation
                |
                v
Generated CSV result files
                |
                v
Independent Python validation
```

## Project structure

```text
ev-thermal-controller/
├── cpp/
│   ├── include/
│   │   └── thermal_controller.hpp
│   ├── src/
│   │   ├── main.cpp
│   │   └── thermal_controller.cpp
│   ├── tests/
│   │   └── test_therm_controller.cpp
│   └── Makefile
├── python/
│   └── scripts/
│       └── validate_scenarios.py
├── docs/
│   ├── requirements_traceability.md
│   └── ...
├── data/
│   ├── scenario_results.csv      # Generated and ignored by Git
│   └── periodic_results.csv      # Generated and ignored by Git
└── README.md
```

## Requirements

To build and run the project, install:

- A C++20-compatible compiler, such as `clang++` or `g++`
- `make`
- Python 3
- Git, if cloning or contributing to the repository

The C++ Makefile builds with strict compiler warnings:

```text
-Wall -Wextra -pedantic
```

## Quick start

Clone the repository and enter the project directory:

```bash
git clone <repository-url>
cd ev-thermal-controller
```

Run the C++ unit tests:

```bash
cd cpp
make test
```

Run the C++ scenario runner and periodic simulation:

```bash
make demo
```

Run complete automated validation:

```bash
make validate
```

Return to the repository root when needed:

```bash
cd ..
```

## Testing and validation

### C++ unit tests

Run:

```bash
cd cpp
make test
```

Current validated result:

```text
Summary: 14/14 tests passed.
```

The C++ unit tests cover:

- Invalid temperature-sensor handling
- Missing CAN-style message handling
- CAN-style message timeout handling
- Below-range and above-range temperature faults
- Normal-operation boundaries
- Cooling-command calculation
- Charging-derate behavior
- Critical-overtemperature behavior
- Safety-fault priority

### C++ scenario runner

Run:

```bash
cd cpp
make demo
```

The scenario runner prints readable output and creates fresh CSV logs:

```text
data/scenario_results.csv
data/periodic_results.csv
```

The generated CSV files are ignored by Git because they are recreated each time the demo or validator runs.

### Python validation

Run:

```bash
cd cpp
make validate
```

The Python validator:

1. Runs the C++ demo to create fresh CSV results.
2. Validates seven named controller scenarios.
3. Validates seven periodic controller samples.
4. Confirms every periodic sample is spaced exactly 100 ms apart.
5. Returns a non-zero exit code if any result is missing or incorrect.

Current validated result:

```text
Scenario summary: 7/7 scenarios passed.
Periodic summary: 7/7 periodic steps passed.
[PASS] All periodic controller updates are spaced by 100 ms
[PASS] Complete validation passed.
```

## Safety and scope

This repository is an educational simulation, not production vehicle software.

It does not claim:

- Real CAN, LIN, Ethernet, or vehicle-network communication
- Real-time operating-system behavior
- Hardware-in-the-loop testing
- Functional-safety compliance, including ISO 26262 compliance
- AUTOSAR compliance
- Production calibration, control policy, or thermal-model accuracy
- Approval by any automotive manufacturer or organization

All thresholds, sensor inputs, fault decisions, controller outputs, timing values, and tests are simplified examples for learning and portfolio demonstration.

## Future improvements

Potential future enhancements include:

- A stateful fault-latching and recovery model
- Hysteresis around temperature-mode thresholds
- CMake build support in addition to the Makefile
- A dedicated test-results document or screenshots
- Optional visualization of generated periodic CSV data
- More realistic simulated CAN-message structures and communication behavior
- Additional unit tests for boundary and recovery conditions

## Author

Hamed Safaei  
Computer Science Student, University of Calgary

Interests: embedded software, automotive systems, vehicle diagnostics, EV controls, and automated testing.
