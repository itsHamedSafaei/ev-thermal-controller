# System Requirements

## 1. Purpose

This project simulates a simplified electric-vehicle battery thermal-control system.

The simulated controller receives battery temperature, ambient temperature, charging-current, and message-status inputs. It calculates a cooling command, selects an operating mode, and applies safe behavior when the temperature is unsafe or sensor/message data is unreliable.

This is an educational software project only. It does not connect to, monitor, or control a real vehicle, battery, charger, ECU, or CAN network.

## 2. System Inputs

The controller shall receive these simulated inputs:

| Input | Type | Unit | Description |
|---|---:|---|---|
| Battery temperature | `float` | °C | Simulated battery temperature |
| Ambient temperature | `float` | °C | Simulated outside temperature |
| Charge current | `float` | A | Simulated current flowing into the battery while charging |
| Temperature sensor valid | `bool` | N/A | Indicates whether the temperature reading is trustworthy |
| CAN message received | `bool` | N/A | Indicates whether required simulated vehicle data was received |
| Message age | `int` | ms | Time since the last required simulated message was received |

## 3. System Outputs

The controller shall produce these simulated outputs:

| Output | Type | Unit | Description |
|---|---:|---|---|
| Cooling command | `float` | % | Requested cooling output from 0% to 100% |
| Vehicle mode | `enum` | N/A | Current operating mode |
| Charging enabled | `bool` | N/A | Whether simulated charging is allowed |
| Fault active | `bool` | N/A | Indicates whether the controller has detected a fault |
| Fault reason | `string` | N/A | Human-readable explanation of the detected fault |

## 4. Operating Modes

The controller shall use the following operating modes:

| Mode | Meaning | Expected behavior |
|---|---|---|
| `NORMAL` | Battery condition is safe | Charging enabled; cooling is based on temperature |
| `COOLING` | Battery is above the target temperature but not unsafe | Charging enabled; cooling command increases |
| `DERATE_CHARGING` | Battery is hot enough to require reduced charging | Charging limited; cooling command is 100% |
| `FAULT` | Temperature, sensor data, or messages are unsafe/unreliable | Charging disabled; cooling command is 100%; fault reason recorded |

## 5. Functional Requirements

### FR-01: Temperature target

The controller shall use a battery-temperature target of 30°C for the initial simulation.

### FR-02: Cooling command

When battery temperature is above 30°C and below 50°C, the controller shall increase the cooling command as temperature rises.

The cooling command shall always remain between 0% and 100%.

### FR-03: Normal mode

When battery temperature is at or below 30°C, the temperature sensor is valid, and required simulated messages are current, the controller shall enter `NORMAL` mode.

In `NORMAL` mode, simulated charging shall remain enabled.

### FR-04: Cooling mode

When battery temperature is above 30°C but below 50°C, the temperature sensor is valid, and required simulated messages are current, the controller shall enter `COOLING` mode.

In `COOLING` mode, simulated charging shall remain enabled.

### FR-05: Derate charging mode

When battery temperature is at or above 50°C but below 60°C, the controller shall enter `DERATE_CHARGING` mode.

In `DERATE_CHARGING` mode, the controller shall command 100% cooling and indicate that simulated charging should be reduced.

### FR-06: Critical over-temperature fault

When battery temperature is at or above 60°C, the controller shall enter `FAULT` mode.

In `FAULT` mode, simulated charging shall be disabled, cooling shall be 100%, and the fault reason shall indicate critical over-temperature.

### FR-07: Invalid temperature sensor fault

When the temperature sensor is invalid, the controller shall enter `FAULT` mode.

In `FAULT` mode, simulated charging shall be disabled, cooling shall be 100%, and the fault reason shall indicate invalid temperature-sensor data.

### FR-08: Temperature-range validation

When the reported battery temperature is below -40°C or above 120°C, the controller shall treat the reading as invalid and enter `FAULT` mode.

### FR-09: Missing-message fault

When a required simulated CAN message is not received, the controller shall enter `FAULT` mode.

### FR-10: Message-timeout fault

When the age of a required simulated message is more than 500 ms, the controller shall enter `FAULT` mode.

### FR-11: Fault priority

Safety-related faults shall take priority over normal operating behavior.

For example, if battery temperature is 65°C and the simulated sensor is invalid, the controller shall enter `FAULT` mode rather than `DERATE_CHARGING` mode.

### FR-12: Output logging

For every controller update, the system shall log the simulated inputs, vehicle mode, cooling command, charging-enabled status, fault-active status, and fault reason.

## 6. Non-Functional Requirements

### NFR-01: Periodic execution

The controller should run as a simulated periodic task with an initial update interval of 100 ms.

### NFR-02: Code quality

The C++ source code shall be organized into modular components and use descriptive names.

### NFR-03: Testability

The controller logic shall be designed so that normal, boundary, and fault conditions can be tested automatically.

### NFR-04: Documentation

The repository shall include documentation for requirements, design assumptions, safety scope, testing, and instructions for running the project.

### NFR-05: Scope limitation

The project shall remain a simulation only and shall not connect to a real vehicle or transmit any commands to a real vehicle network.

## 7. Initial Simulation Assumptions

The following values are chosen for education and testing only. They are not real Rivian, Volkswagen, battery, ECU, or production-vehicle specifications.

| Condition | Expected behavior |
|---|---|
| Battery temperature ≤ 30°C | `NORMAL`; cooling 0%; charging enabled |
| Battery temperature > 30°C and < 50°C | `COOLING`; cooling increases with temperature; charging enabled |
| Battery temperature ≥ 50°C and < 60°C | `DERATE_CHARGING`; cooling 100%; charging reduced |
| Battery temperature ≥ 60°C | `FAULT`; cooling 100%; charging disabled |
| Temperature sensor invalid | `FAULT`; cooling 100%; charging disabled |
| Temperature below -40°C or above 120°C | `FAULT`; cooling 100%; charging disabled |
| Required simulated CAN message missing | `FAULT`; cooling 100%; charging disabled |
| Required simulated message older than 500 ms | `FAULT`; cooling 100%; charging disabled |