# Firmware

This directory contains the embedded firmware implementations for the **IoT RFID & Keypad Access Control System**.

The firmware demonstrates embedded system design, hardware–software integration, and access control logic for a physical locking mechanism using RFID authentication and keypad-based fallback.

Two firmware variants are provided to reflect different hardware capabilities.

---

## Firmware Variants

### ESP32 Firmware (Wi-Fi Enabled)

**Path**

firmware/esp32/esp32_access_control.ino

**Purpose**
A network-capable firmware implementation supporting both **local physical authentication** and **remote control via a web interface**.

**Key Capabilities**
- RFID UID-based authentication
- Keypad passcode fallback
- LCD status feedback with simple animations
- Servo-based lock and unlock control
- Wi-Fi connectivity
- HTTP endpoints for remote lock/unlock
- Automatic re-lock after configurable timeout
- Offline fallback when Wi-Fi is unavailable

**Web Endpoints (Demo)**
- `GET /` – Device status page
- `GET /status` – JSON lock state and device info
- `POST /unlock?token=...` – Remote unlock
- `POST /lock?token=...` – Remote lock

> Note: The web API uses a simple token mechanism intended for demonstration only.  
> It is not designed for production security use.

---

### Arduino Firmware (Offline / Embedded-Only)

**Path**

firmware/arduino/arduino_access_control.ino


**Purpose**
A fully offline embedded implementation focusing on **deterministic access control logic** without network dependencies.

**Key Capabilities**
- RFID UID allowlist validation
- Keypad passcode authentication
- LCD feedback for system states
- Servo-controlled locking mechanism
- Automatic re-lock timing
- Input debouncing and basic failure handling

This variant represents the **core embedded logic** of the system and mirrors the physical behavior demonstrated during the exhibition.

---

## Authentication Logic Overview

Both firmware variants follow the same logical flow:

1. System initializes in **LOCKED** state
2. User attempts authentication via:
   - RFID card, or
   - Keypad passcode
3. Credentials are validated locally in firmware
4. LCD displays authentication result
5. Servo unlocks on success
6. System automatically re-locks after a timeout

The ESP32 variant extends this flow with optional **remote control via Wi-Fi**.

---

## Hardware Interfaces Used

- MFRC522 RFID reader (SPI)
- 4×4 matrix keypad
- 16×2 LCD display (I2C)
- Servo motor (PWM)
- Microcontroller (ESP32 or Arduino-class)

Pin assignments are documented directly within each firmware file.

---

## Development Notes

- The original exhibition firmware is not preserved.
- The code in this directory is a **clean reimplementation** based on the demonstrated system behavior.
- Emphasis is placed on:
  - Readability
  - Clear state handling
  - Inline documentation
  - Interview-ready structure

Future improvements could include:
- Secure credential storage (EEPROM / NVS)
- Encrypted communication for web control
- Event logging to external storage or cloud services

---

## Disclaimer

These firmware implementations are intended for **academic and demonstrative purposes only**.  
They are not production-grade security systems and should not be deployed in real-world access control environments without significant hardening.


