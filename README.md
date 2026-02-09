\# IoT RFID \& Keypad Access Control System



An embedded IoT access control prototype implementing RFID-based authentication with keypad passcode fallback, LCD status display, and an electromechanical locking mechanism.



This system was \*\*designed, built, and demonstrated as a working hardware prototype\*\* at the LNBTI Talent Show \& IoT Exhibition (2023).



\## Overview

The project simulates a secure entry system where users authenticate using an RFID card. If RFID authentication fails or is unavailable, a keypad-based passcode provides a controlled override. System state and access results are displayed via an LCD, while a servo-based locking mechanism physically controls access.



\## Key Capabilities

\- RFID UID-based authentication

\- Keypad passcode fallback

\- LCD feedback (Prompt / Granted / Denied)

\- Servo/lock actuation

\- Embedded control logic and state handling

\- Physical system demonstration and validation



\## System Architecture

!\[Architecture](docs/diagrams/system-architecture.png)



\## Hardware Components

\- Microcontroller (ESP32 / Arduino-class)

\- MFRC522 RFID Reader

\- 4x4 Keypad

\- 16x2 LCD Display

\- Servo motor / locking latch

\- Power supply and discrete components



\## Prototype Photos

\*\*System Overview\*\*  

!\[Overview](docs/photos/01-system-overview.jpg)



\*\*Keypad \& LCD Interface\*\*  

!\[Keypad](docs/photos/02-keypad-lcd.jpg)



\*\*RFID Module\*\*  

!\[RFID](docs/photos/03-rfid-module.jpg)



\*\*Locking Mechanism\*\*  

!\[Lock](docs/photos/04-lock-mechanism.jpg)



\## Public Demonstration

This prototype was demonstrated at:



\*\*LNBTI Talent Show \& IoT Exhibition – 2023\*\*



!\[Exhibition](docs/exhibition/05-exhibition-demo.jpg)

!\[Team](docs/exhibition/06-team-presentation.jpg)



\## Firmware Notes

The firmware for this project handled:

\- RFID UID validation

\- Keypad input parsing

\- LCD state updates

\- Timed lock/unlock control

\- Failure handling and retries



> Source code is not included in this repository.  

> This repository focuses on \*\*system design, hardware integration, and documented implementation\*\*.



\## Skills Demonstrated

\- Embedded systems design

\- Hardware-software integration

\- RFID-based authentication logic

\- Access control workflows

\- Technical documentation

\- Public technical presentation



\## Disclaimer

This project is a \*\*prototype and academic demonstration\*\*, not a production security system.



