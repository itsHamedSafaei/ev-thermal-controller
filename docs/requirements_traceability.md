# Requirements Traceability Matrix

## 1. Purpose

This document maps system requirements to the current implementation and test evidence for the EV Battery Thermal Controller and CAN Test Simulator.

The traceability matrix helps confirm that requirements are not only documented, but also implemented and tested. It also identifies planned or partially implemented work so that the project scope remains accurate and honest.

## 2. Status Definitions

| Status | Meaning |
|---|---|
| `Implemented` | The requirement is implemented and has supporting test or demonstration evidence |
| `Partially Implemented` | Some of the requirement is implemented, but additional work or tests are still needed |
| `Planned` | The requirement is documented but has not been implemented yet |

## 3. Functional Requirements

| ID | Requirement summary | Implementation evidence | Test or demonstration evidence | Status |
|---|---|---|---|---|
| FR-01 | Use a battery-temperature target of 30°C | `k_target_temperature_c = 30.0` in `cpp/src/thermal_controller.cpp` | Unit tests verify 30.0°C remains `NORMAL` and 30.1°C begins `COOLING` | `Implemented` |
| FR-02 | Increase cooling above 30°C and limit it from 0% to 100% | `calculate_cooling_command()` uses a proportional calculation and `std::clamp()` | Unit tests verify 0.0%, 0.5%, 60.0%, 99.5%, and 100.0% cooling behavior | `Implemented` |
| FR-03 | Enter `NORMAL` mode at or below 30°C with valid current inputs | `update_controller()` returns `VehicleMode::Normal` when temperature is at or below the target | Unit tests verify 25.0°C and 30.0°C `NORMAL` behavior | `Implemented` |
| FR-04 | Enter `COOLING` mode above 30°C and below 50°C | `update_controller()` returns `VehicleMode::Cooling` in the simulated cooling range | Unit tests verify 30.1°C, 42.0°C, and 49.9°C cooling behavior | `Implemented` |
| FR-05 | Enter `DERATE_CHARGING` mode from 50°C to below 60°C | `update_controller()` returns `VehicleMode::DerateCharging`, 100% cooling, and `charging_derated = true` | Unit tests verify 50.0°C and 59.9°C derate behavior; scenario runner demonstrates 55.0°C | `Implemented` |
| FR-06 | Enter a fault at or above 60°C | `update_controller()` returns a safe `FAULT` output with `CriticalOvertemperature` | Unit test verifies 60.0°C; scenario runner and Python validator verify the critical-overtemperature case | `Implemented` |
| FR-07 | Enter a fault when the temperature sensor is invalid | `update_controller()` returns a safe `FAULT` output with `InvalidTemperatureSensor` | C++ unit test, scenario runner, and Python validator verify the invalid-sensor case | `Implemented` |
| FR-08 | Treat temperatures below -40°C or above 120°C as invalid | `update_controller()` checks the simulated valid range from -40°C to 120°C | Unit tests verify a below-range value of -41.0°C and an above-range value of 200.0°C | `Implemented` |
| FR-09 | Enter a fault when a required simulated CAN-style message is missing | `update_controller()` checks `can_message_received` | C++ unit test verifies a missing CAN-style message produces `FAULT` | `Implemented` |
| FR-10 | Enter a fault when a required simulated message is older than 500 ms | `update_controller()` checks `message_age_ms > 500` | C++ unit test and Python scenario validation verify a 501 ms CAN timeout | `Implemented` |
| FR-11 | Prioritize safety-related faults over normal operating behavior | `update_controller()` validates data before normal mode and temperature decisions | C++ unit test verifies invalid sensor data takes priority over a 65.0°C value | `Implemented` |
| FR-12 | Log controller inputs and outputs for every simulated scenario update | `cpp/src/main.cpp` writes scenario inputs and controller outputs to `data/scenario_results.csv` | `make demo` creates CSV results; Python validator reads and validates the generated CSV file | `Partially Implemented` |

## 4. Non-Functional Requirements

| ID | Requirement summary | Implementation evidence | Test or demonstration evidence | Status |
|---|---|---|---|---|
| NFR-01 | Run as a simulated periodic task with an initial update interval of 100 ms | Current scenario runner executes predefined scenarios one time each | No periodic 100 ms simulation sequence exists yet | `Planned` |
| NFR-02 | Use modular, descriptive C++ code | Controller interface, controller implementation, scenario runner, and unit tests are separated into focused files | Code is built using strict warnings: `-Wall -Wextra -pedantic` | `Implemented` |
| NFR-03 | Support automated testing of normal, boundary, and fault conditions | Controller returns `ControllerOutput`, allowing tests to inspect all outputs | 13 C++ unit tests and 7 Python CSV scenario validations currently pass | `Implemented` |
| NFR-04 | Include requirements, design assumptions, safety scope, testing, and run instructions | Requirements, safety notes, and architecture documents are present | Documentation is being expanded; README update and test-results documentation are still planned | `Partially Implemented` |
| NFR-05 | Remain simulation only and do not communicate with a real vehicle network | All project inputs are simulated in source code; no hardware or CAN-library integration is used | Safety notes and source-code scope statements describe the simulation-only boundary | `Implemented` |

## 5. Current Test Evidence

### 5.1 C++ Unit Tests

The C++ unit-test executable validates the controller directly.

Run:

```bash
cd cpp
make test
```

Current expected result:

```text
Summary: 13/13 tests passed.
```

The current C++ test suite covers:

- Invalid temperature sensor
- Missing CAN-style message
- CAN-style message timeout
- Above-range temperature
- Normal operation
- Temperature boundaries
- Cooling calculation
- Charging derate behavior
- Critical overtemperature
- Fault-priority behavior

### 5.2 Scenario Runner

The C++ scenario runner provides readable terminal output and generated CSV logging.

Run:

```bash
cd cpp
make demo
```

The scenario runner currently demonstrates:

- Normal operation
- Cooling required
- Charging derate
- Invalid temperature sensor
- CAN message timeout
- Out-of-range temperature
- Critical overtemperature

### 5.3 Python Validation

The Python validator independently checks the scenario CSV results.

Run:

```bash
cd cpp
make validate
```

Current expected result:

```text
Summary: 7/7 scenarios passed.
```

## 6. Remaining Planned Work

The following improvements are planned to close the remaining partial or planned requirements:

| Item | Reason |
|---|---|
| Add a 100 ms periodic simulation sequence | Addresses NFR-01 periodic-execution requirement |
| Expand logging evidence or document the scenario-level logging boundary | Clarifies the scope of FR-12 logging behavior |
| Update README.md | Makes the GitHub landing page accurately reflect completed features and commands |
| Add a test-results document or screenshots | Makes validation evidence easier for GitHub reviewers to inspect |
| Add CMake build support | Provides a more standard C++ build configuration in addition to the Makefile |
| Add optional dashboard visualization | Provides a polished portfolio interface after the technical core is complete |

## 7. Scope Reminder

This matrix describes an educational simulation only.

The project does not claim production-vehicle capability, real CAN communication, ISO 26262 compliance, AUTOSAR compliance, real-time operating-system behavior, hardware-in-the-loop testing, or approval by Rivian, Volkswagen Group, or any other organization.

All thresholds, inputs, outputs, controller decisions, and test cases are simplified examples for learning and portfolio development.