# Software Architecture

## 1. Purpose

This document describes the software architecture of the EV Battery Thermal Controller and CAN Test Simulator.

The project is an educational simulation of a simplified electric-vehicle battery thermal-control system. It receives simulated sensor and CAN-style message-status inputs, calculates a cooling command, selects an operating mode, and applies simplified safe-state behavior when input data is invalid, missing, stale, or unsafe.

The project does not connect to, monitor, control, or communicate with any real vehicle, battery, charger, ECU, or CAN network.

## 2. Architecture Overview

```text
Simulated battery, sensor, and CAN-style inputs
                |
                v
      C++ Thermal Controller
                |
                v
         ControllerOutput
  - Vehicle mode
  - Cooling command
  - Charging enabled
  - Charging derated
  - Fault active
  - Fault code
                |
                v
      C++ Scenario Runner
                |
                +--------------------------+
                |                          |
                v                          v
    Human-readable terminal output   CSV result logging
                                          |
                                          v
                          data/scenario_results.csv
                                          |
                                          v
                         Python CSV Scenario Validator
                                          |
                                          v
                         PASS / FAIL automation summary
```

## 3. Main Components

| Component | Location | Responsibility |
|---|---|---|
| Controller interface | `cpp/include/thermal_controller.hpp` | Defines controller data types, operating modes, fault codes, inputs, outputs, and function declarations |
| Controller logic | `cpp/src/thermal_controller.cpp` | Validates simulated inputs, calculates cooling, selects operating modes, and returns a complete controller decision |
| Scenario runner | `cpp/src/main.cpp` | Runs predefined simulated scenarios, prints results to the terminal, and writes results to CSV |
| C++ unit tests | `cpp/tests/test_therm_controller.cpp` | Tests normal operation, temperature boundaries, faults, and fault-priority behavior |
| Build workflow | `cpp/Makefile` | Provides `make test`, `make demo`, `make validate`, and `make clean` commands |
| CSV log | `data/scenario_results.csv` | Generated file containing inputs and controller outputs for each scenario |
| Python validator | `python/scripts/validate_scenarios.py` | Runs the C++ scenario runner, reads the CSV log, and verifies expected scenario outputs |
| Requirements | `docs/requirements.md` | Defines the intended system behavior and simulation assumptions |
| Safety scope | `docs/safety_notes.md` | Defines simulation-only limits and simplified safety-oriented concepts |

## 4. C++ Controller Design

The main controller function is:

```cpp
ControllerOutput update_controller(const SensorData& data);
```

The function receives one complete set of simulated input data and returns one complete controller decision.

### 4.1 Simulated Inputs

```cpp
struct SensorData {
    double battery_temperature_c;
    double ambient_temperature_c;
    double charge_current_a;
    bool temperature_sensor_valid;
    bool can_message_received;
    int message_age_ms;
};
```

The current simulation uses battery temperature, sensor validity, CAN-message availability, and message age to make the controller decision.

Ambient temperature and charge current are included in the simulated input and logged to CSV. They are reserved for future controller-model extensions.

### 4.2 Controller Outputs

```cpp
struct ControllerOutput {
    VehicleMode mode;
    double cooling_command_percent;
    bool charging_enabled;
    bool charging_derated;
    bool fault_active;
    FaultCode fault_code;
};
```

The controller returns all simulated decisions together. This makes the controller easier to test because a test can validate mode, cooling command, charging state, and fault information from a single function call.

### 4.3 Operating Modes

| Mode | Simulated behavior |
|---|---|
| `NORMAL` | Battery temperature is at or below 30°C; charging is enabled; cooling is 0% |
| `COOLING` | Battery temperature is above 30°C and below 50°C; charging is enabled; cooling increases with temperature |
| `DERATE_CHARGING` | Battery temperature is from 50°C to below 60°C; cooling is 100%; charging remains enabled but is marked as derated |
| `FAULT` | Simulated input data is invalid, missing, stale, out of range, or critically hot; cooling is 100%; charging is disabled |

### 4.4 Fault Priority

The controller validates input data before selecting normal operating behavior.

The simplified fault priority order is:

1. Invalid temperature sensor
2. Missing CAN-style message
3. CAN-style message timeout
4. Temperature outside the valid simulation range
5. Critical overtemperature
6. Normal temperature-based operating modes

For example, if the reported temperature is 65°C but the temperature sensor is marked invalid, the controller returns:

```text
Mode: FAULT
Fault code: INVALID_TEMPERATURE_SENSOR
```

The controller does not treat an invalid sensor reading as trustworthy data.

### 4.5 Cooling Calculation

For valid battery temperatures above 30°C and below 50°C, cooling uses a simplified proportional calculation:

```text
cooling command = proportional gain × (battery temperature - target temperature)
```

The initial simulation uses:

```text
Target temperature: 30°C
Proportional gain: 5
Cooling command limit: 0% to 100%
```

Example:

```text
Battery temperature: 42°C
Temperature error: 42 - 30 = 12°C
Cooling command: 5 × 12 = 60%
```

## 5. Scenario Runner and CSV Logging

The C++ scenario runner is located in:

```text
cpp/src/main.cpp
```

It runs predefined simulation cases:

- Normal operation
- Cooling required
- Charging derate
- Invalid temperature sensor
- CAN message timeout
- Out-of-range temperature
- Critical overtemperature

For every scenario, the program:

1. Creates simulated `SensorData`.
2. Calls `update_controller`.
3. Prints simulated inputs and controller outputs to the terminal.
4. Writes the same inputs and outputs to `data/scenario_results.csv`.

The CSV file is generated output and is ignored by Git.

## 6. Testing and Automation

### 6.1 C++ Unit Tests

The C++ unit-test program validates normal behavior, boundary conditions, fault behavior, safe-state outputs, and fault priority.

Current coverage includes:

- Invalid sensor fault
- Missing CAN-style message fault
- CAN-style message timeout fault
- Temperature out-of-range fault
- Normal operation at 25°C
- Normal boundary at 30°C
- Cooling boundary at 30.1°C
- Cooling behavior at 42°C
- Cooling boundary at 49.9°C
- Charging derate at 50°C
- Charging derate at 59.9°C
- Critical overtemperature at 60°C
- Invalid-sensor fault priority

Run the C++ tests with:

```bash
cd cpp
make test
```

### 6.2 Python Scenario Validation

The Python validator provides an additional automation layer.

It performs the following steps:

1. Runs the C++ scenario runner through `make demo`.
2. Reads the generated CSV result file.
3. Checks that all expected scenarios are present.
4. Checks that no unexpected scenarios are present.
5. Compares mode, cooling command, charging state, derate state, fault state, and fault code against expected values.
6. Prints a PASS or FAIL result for every scenario.

Run the complete Python validation workflow with:

```bash
cd cpp
make validate
```

## 7. Build and Run Commands

Run these commands from the `cpp` directory:

```bash
make test
```

Builds and runs the C++ unit tests.

```bash
make demo
```

Builds and runs the C++ scenario runner. It also creates the generated CSV log:

```text
data/scenario_results.csv
```

```bash
make validate
```

Runs the Python validation workflow. The Python script runs the C++ scenario runner, reads the generated CSV file, and checks all scenario results.

```bash
make clean
```

Removes generated C++ executables and old object files.

## 8. Design Decisions

| Design decision | Reason |
|---|---|
| Use C++ for controller logic | Demonstrates structured, deterministic controller logic relevant to embedded and controls software |
| Use enums for modes and fault codes | Makes valid states explicit and easy to compare in automated tests |
| Return `ControllerOutput` | Keeps all controller decisions together and makes test assertions clearer |
| Use `const char*` for readable mode and fault text | Supports readable logging without using dynamically constructed error strings |
| Separate controller logic from terminal output | Allows the same controller function to be used by tests, the scenario runner, and future tools |
| Write results to CSV | Creates a simple machine-readable interface between C++ and Python |
| Use Python standard-library validation | Avoids external dependencies while demonstrating automation and CSV analysis |
| Keep generated logs out of Git | Tracks source code and documentation rather than generated result files |

## 9. Scope and Limitations

This project is an educational simulation and portfolio project.

It does not:

- Connect to a real vehicle, battery, charger, ECU, or CAN bus
- Implement production CAN communication
- Implement real-time scheduling or a real operating system
- Claim ISO 26262, AUTOSAR, Rivian, Volkswagen Group, SAE, or other production compliance
- Provide real vehicle diagnostics, safety decisions, or charging control
- Replace manufacturer procedures, professional automotive service, validation, or safety engineering

All thresholds, inputs, outputs, messages, faults, and behaviors are simplified examples selected for education and testing.