# Safety Notes and Project Scope

## 1. Purpose of this document

This document defines the safety boundaries and educational scope of the EV Battery Thermal Controller and CAN Test Simulator project.

The project is intended to demonstrate software-engineering concepts relevant to vehicle controls, including input validation, fault detection, safe-state behavior, testing, and documentation.

## 2. Educational simulation only

This repository contains an educational software simulation.

It does not connect to, monitor, control, modify, reprogram, or communicate with:

- A real vehicle
- A real electric-vehicle battery
- A real battery-management system
- A real charging system
- A real electronic control unit (ECU)
- A real Controller Area Network (CAN) bus
- Any production automotive hardware

All temperature values, current values, CAN-style messages, fault conditions, cooling commands, and operating thresholds are simulated examples.

## 3. No production or safety certification claim

This project is not production automotive software.

It does not claim compliance with ISO 26262, AUTOSAR, SAE standards, Rivian standards, Volkswagen Group standards, or any other automotive safety, quality, cybersecurity, regulatory, or certification requirement.

The project uses simplified safety-oriented concepts only for learning and portfolio purposes.

## 4. Safety-oriented concepts demonstrated

The project will demonstrate simplified examples of the following concepts:

- Checking whether sensor data is valid
- Checking whether required messages were received recently
- Detecting impossible or unsafe simulated temperature values
- Recording a fault reason
- Entering a safer operating state after a simulated fault
- Disabling simulated charging during a critical simulated fault
- Commanding maximum simulated cooling during a critical simulated fault
- Testing normal, boundary, failure, and recovery scenarios

## 5. Simplified safe-state behavior

For this simulation, a safe state means:

- The controller enters `FAULT` mode
- Simulated charging is disabled
- Simulated cooling is commanded to 100%
- A fault reason is recorded for logging and testing

This is a simplified learning model. A real EV safety strategy would require extensive engineering, hardware design, redundancy analysis, validation, safety cases, regulatory review, and production testing.

## 6. Prohibited use

Do not use this project to:

- Control, repair, tune, modify, or reprogram a real vehicle
- Send commands to a real CAN bus
- Alter battery, charging, braking, steering, airbag, lighting, emissions, immobilizer, or other vehicle systems
- Make decisions about real vehicle safety
- Diagnose a real battery or vehicle fault
- Replace professional automotive service, manufacturer procedures, or safety testing

## 7. Responsible development approach

The project will follow these responsible-development practices:

1. Keep the project limited to simulated data and simulated outputs.
2. Use version control with Git and GitHub.
3. Document requirements before implementation.
4. Write automated tests for normal and fault conditions.
5. Clearly label assumptions and simulated thresholds.
6. Avoid publishing secrets, private keys, credentials, or personal information.
7. Maintain clear documentation so that reviewers understand the project scope and limitations.

## 8. Relation to functional safety

Real automotive functional safety concerns reducing unacceptable risk caused by malfunctioning electrical and electronic systems in road vehicles.

This project does not implement or certify ISO 26262. However, it introduces a small number of related engineering habits:

- Define expected behavior before implementation
- Validate data before trusting it
- Consider missing, delayed, invalid, and out-of-range inputs
- Define a safer fallback response
- Test fault cases, not only normal cases
- Record the reason for a detected fault

## 9. Disclaimer

This repository is provided for educational and portfolio purposes only.

Use of this code is entirely at the user’s own risk. The author makes no warranty that the software is accurate, complete, safe, reliable, suitable for a particular purpose, or appropriate for any real-world automotive application.