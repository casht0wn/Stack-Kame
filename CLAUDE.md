# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Stack Kame is a quadruped robot controller based on the miniKame design. It consists of two independent PlatformIO firmware projects:

- **`firmware/legs-esp32s3/`** — Robot leg controller running on Adafruit Feather ESP32-S3 (no PSRAM), controlling 8x MG90S servos via a PCA9685 PWM driver over I2C
- **`firmware/controller-cardputer/`** — Handheld remote running on M5Stack Cardputer (M5Stack StampS3), communicating over ESP-NOW wireless protocol

CAD files live in `cad/` (FreeCAD `.fcstd`, `.step`, `.stl` exports, and a parametric OpenSCAD servo horn).

## Build & Flash Commands

Both firmware projects use PlatformIO. Run commands from within the respective project directory:

```bash
# Build
cd firmware/legs-esp32s3
pio run

# Build and upload
pio run -t upload

# Monitor serial output (115200 baud)
pio device monitor

# Same commands apply for the controller
cd firmware/controller-cardputer
pio run -t upload
pio device monitor
```

PlatformIO has no test runner configured — the `test/` directories are empty stubs.

## Architecture

### Communication Protocol

Commands are single bytes sent over ESP-NOW from controller to robot. Key commands (defined in `firmware/controller-cardputer/include/commands.h`):

| Hex  | Action              |
|------|---------------------|
| 0x01 | STOP                |
| 0x02 | WALK_FORWARD        |
| 0x03 | WALK_BACKWARD       |
| 0x04 | TURN_LEFT           |
| 0x05 | TURN_RIGHT          |
| 0x06 | MOONWALK_FORWARD    |
| 0x07 | MOONWALK_BACKWARD   |
| 0x08 | JUMP                |
| 0x09 | SHUFFLE_LEFT        |
| 0x0A | SHUFFLE_RIGHT       |
| 0x0B | HOME                |
| 0x0C | ZERO (all servos to 90°) |
| 0x0D | CAL_SELECT          |
| 0x0E | CAL_JOG             |
| 0xFF | EMERGENCY_STOP (immediate interrupt) |

See `docs/PROTOCOL.md` for full protocol details including timing, command sequences, and debugging.

### Robot Legs Firmware (`firmware/legs-esp32s3/`)

- **`src/main.cpp`** — ESP-NOW receiver, 16-entry FIFO command queue, NVS calibration storage (via `Preferences`), serial debug commands
- **`src/stackkame.cpp`** / **`include/stackkame.h`** — `StackKame` class: servo mapping, movement functions, oscillator coordination
- **`lib/Octosnake/Oscillator.h/.cpp`** — Sinusoidal oscillator driving each servo; configurable amplitude, offset, phase, and period

**Servo mapping** (PCA9685 channel order): FL_HIP(0), FR_HIP(1), FL_FOOT(2), FR_FOOT(3), BL_HIP(4), BR_HIP(5), BL_FOOT(6), BR_FOOT(7). Servos FL_HIP, FL_FOOT, BR_HIP, BR_FOOT are reversed. PWM range: SERVOMIN=205 (~1000µs), SERVOMAX=410 (~2000µs) at 50Hz.

Per-servo calibration trims are stored in NVS and applied on top of oscillator outputs.

### Controller Firmware (`firmware/controller-cardputer/`)

- **`src/main.cpp`** — Keyboard input loop, screen state management, dispatches to screen renderers
- **`src/screens.cpp`** — Four screens: Menu, Controls, Diagnostics (battery %, RSSI, scrolling log), Calibration (per-servo angle adjustment)
- **`src/communication.cpp`** / **`include/communication.h`** — `CommandSender` class wrapping ESP-NOW send
- **`include/app_state.h`** — `AppState` struct holding all UI state and enums for active screen
- **`include/screens.h`** — Screen rendering function declarations

Number keys 0–9 set movement step count; Escape triggers emergency stop.
