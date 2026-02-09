# IoT RFID & Keypad Access Control System

Embedded IoT access control prototype implementing RFID-based authentication, keypad passcode fallback, real-time LCD feedback, and an electromechanical locking mechanism.

This project was designed, built, and publicly demonstrated as a working hardware prototype at the **LNBTI Talent Show & IoT Exhibition (2023)**.

## Table Of Contents
- Why It Matters
- System Capabilities
- Workflow Overview
- System Architecture
- Hardware Components
- Prototype Documentation
- Firmware Notes
- Project Status
- Credits And Attribution
- Disclaimer

## Why It Matters
Physical access control systems are widely used in offices, labs, and restricted environments. This project demonstrates how embedded systems can combine multiple authentication methods with clear user feedback and deterministic control logic to manage secure physical access.

The emphasis is on **hardware–software integration**, **state-driven firmware design**, and **reliable physical actuation**, rather than cloud connectivity or production deployment.

## System Capabilities
- RFID UID-based authentication
- Keypad-based passcode fallback
- LCD-based system feedback (Idle, Granted, Denied)
- Servo-controlled physical locking mechanism
- Deterministic embedded state handling
- Fully demonstrated physical prototype

## Workflow Overview
- System initializes and enters idle state
- User authenticates via RFID card or keypad
- Credentials are validated in firmware
- LCD displays access result
- Servo unlocks or remains locked
- System resets after a fixed timeout

## System Architecture
- Input Layer: RFID reader, keypad
- Control Layer: Microcontroller firmware (ESP32 / Arduino-class)
- Output Layer: LCD display, servo motor
- Logic Layer: Authentication rules and state machine

![System Architecture](docs/diagrams/system-architecture.png)

## Hardware Components
- Microcontroller (ESP32 / Arduino-class)
- MFRC522 RFID Reader
- 4×4 Matrix Keypad
- 16×2 LCD Display
- Servo motor / locking latch
- Power supply and discrete components

## Prototype Documentation

### System Overview
![System Overview](docs/photos/01-system-overview.jpg)

### Keypad and LCD Interface
![Keypad and LCD](docs/photos/02-keypad-lcd.jpg)

### RFID Module
![RFID Module](docs/photos/03-rfid-module.jpg)

### Locking Mechanism
![Locking Mechanism](docs/photos/04-lock-mechanism.jpg)

## Public Demonstration
This prototype was demonstrated live at:

**LNBTI Talent Show & IoT Exhibition — 2023**

![Exhibition Demo](docs/exhibition/05-exhibition-demo.jpg)  
![Team Presentation](docs/exhibition/06-team-presentation.jpg)

The system was evaluated by academic staff and external visitors as a fully functional physical IoT prototype.

## Firmware Notes
The embedded firmware implemented:
- RFID UID validation
- Keypad input parsing
- LCD state updates
- Timed lock and unlock control
- Failure handling and retry logic

The original exhibition firmware is not preserved in this repository.  
This repository focuses on **system behavior, hardware integration, and documented design**.  
A clean firmware reimplementation may be added in the future.

## Project Status
- Hardware prototype completed
- Successfully demonstrated in a public exhibition
- Firmware reimplementation planned
- Optional extensions: logging, monitoring, or cloud integration

## Credits And Attribution
This project was developed as an academic hardware prototype.

**Contributor**
- Devmith Thenujayan — Embedded Systems Design and Integration

## Disclaimer
This project is an academic hardware prototype developed for demonstration and learning purposes.  
It is not intended for production or commercial security deployment.
