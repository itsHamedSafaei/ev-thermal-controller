# EV Battery Thermal Controller and CAN Test Simulator

An educational software project that simulates an electric-vehicle battery thermal-control system. The project combines C++ controller logic with Python-based test automation, simulated CAN-style vehicle messages, fault detection, and safe-state behavior.

> Status: In progress — Phase 1: project setup and design.

## Why I am building this

I am a Computer Science student at the University of Calgary interested in embedded software, vehicle controls, automotive diagnostics, and electric-vehicle technology. I also have hands-on automotive maintenance and tuning experience.

This project is designed to help me develop practical skills relevant to automotive software teams working on battery management, charging, thermal systems, vehicle controls, and automated testing.

## Planned features

- C++ thermal-control logic for a simulated EV battery
- A periodic control loop that adjusts cooling output
- Vehicle operating modes: `NORMAL`, `COOLING`, `DERATE_CHARGING`, and `FAULT`
- Simulated CAN-style messages for battery temperature, charge current, charger status, and cooling commands
- Fault detection for invalid sensor readings, missing messages, and over-temperature conditions
- Python test automation and log analysis
- C++ unit tests and software-in-the-loop-style test scenarios
- Documentation for system requirements, architecture, assumptions, and test results

## Technology stack

- C++17 or newer
- Python 3
- Git and GitHub
- CMake
- C++ unit tests
- Python testing with pytest
- Simulated CAN messaging

## Planned architecture

```text
Simulated vehicle sensors / CAN messages
                |
                v
      C++ thermal controller
                |
                v
 Cooling and charging-state commands
                |
                v
Python test automation and log analysis
```

## Safety and scope

This is an educational simulation only. It does not connect to, read from, write to, reprogram, or control a real vehicle, battery, ECU, or CAN network.

The thresholds, test cases, and fault responses used in this project are simplified examples for learning. They are not production specifications and are not intended for use in a real vehicle.

## Project roadmap

- [x] Create repository and initial documentation
- [ ] Implement basic C++ thermal controller
- [ ] Add operating modes and fault management
- [ ] Add C++ unit tests
- [ ] Create simulated CAN-style messages
- [ ] Create Python test automation
- [ ] Add software-in-the-loop scenarios
- [ ] Document requirements, architecture, and test results

## Author

Hamed Safaei  
Computer Science Student, University of Calgary  
Interested in embedded software, automotive systems, vehicle diagnostics, and EV controls.