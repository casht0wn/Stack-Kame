# Stack Kame - Quadruped Robot Controller

A modified mini Kame quadruped robot accessory for Stack-Chan, running on Adafruit Feather ESP32-S3 (no PSRAM) with an 8-channel PWM servo wing.

## Hardware

- **Microcontroller**: Adafruit Feather ESP32-S3 (no PSRAM)
- **Servo Controller**: Adafruit 8-channel PWM Servo FeatherWing (PCA9685)
- **Servos**: 8x MG90S digital servos
- **Battery Monitor**: MAX17048 LiPo fuel gauge
- **Physical Design**: Based on miniKame quadruped

## Servo Configuration

```
Servo 0: Front Left Hip
Servo 1: Front Left Foot
Servo 2: Back Left Hip
Servo 3: Back Left Foot
Servo 4: Back Right Hip
Servo 5: Back Right Foot
Servo 6: Front Right Hip
Servo 7: Front Right Foot
```

## Communication Protocols

### 1. ESP-NOW Wireless Control

- **Format**: 2-byte commands
- **Controllers**: M5Stack Cardputer, M5Stack Stack-Chan, or any ESP32 device
- **Pairing**: Auto-accepts commands from any sender (no MAC whitelist)
- **Range**: ~100m line-of-sight (typical ESP-NOW range)

**Command Format**:
- Byte 0: Command type (see command table below)
- Byte 1: Command value (typically number of steps)

**Example (Arduino)**:
```cpp
uint8_t command[2] = {0x02, 5};  // Walk forward 5 steps
esp_now_send(robotMAC, command, 2);
```

### 2. Serial Debugging (USB)

- **Baud Rate**: 115200
- **Interactive Commands**:
  - `h` - Move to home position
  - `z` - Zero all servos (90°)
  - `w` - Walk forward 3 steps
  - `b` - Walk backward 3 steps
  - `t` - Turn left
  - `s` - Emergency stop
  - `r` - Resume from emergency stop
  - `c` - Enter calibration mode
  - `q` - Show queue count

## Command Protocol

| Command | Type | Value | Description |
|---------|------|-------|-------------|
| Stop | 0x01 | 0 | Stop movement and return to home |
| Walk Forward | 0x02 | 1-255 | Walk forward N steps |
| Walk Backward | 0x03 | 1-255 | Walk backward N steps |
| Turn Left | 0x04 | 1-255 | Turn left N steps |
| Turn Right | 0x05 | 1-255 | Turn right N steps |
| Moonwalk Forward | 0x06 | 1-255 | Moonwalk forward N steps |
| Moonwalk Backward | 0x07 | 1-255 | Moonwalk backward N steps |
| Jump | 0x08 | 0 | Perform jump animation |
| Shuffle Left | 0x09 | 1-255 | Lateral shuffle left N steps |
| Shuffle Right | 0x0A | 1-255 | Lateral shuffle right N steps |
| Emergency Stop | 0xFF | 0 | Immediate stop all movements |

## Movement Library

The robot uses an oscillator-based movement system inspired by the original miniKame. Each servo is controlled by a sinusoidal oscillator with configurable:

- **Amplitude**: Range of motion in degrees
- **Offset**: Center position offset
- **Phase**: Phase shift for gait coordination
- **Period**: Time for one complete cycle

### Available Gaits

1. **Walk** - Standard alternating tripod gait for forward/backward movement
2. **Turn** - In-place rotation using differential leg movement
3. **Moonwalk** - Smooth sliding motion (Michael Jackson style!)
4. **Lateral Shuffle** - Side-stepping movement
5. **Jump** - Quick up-down animation

## Command Queue

- **Queue Size**: 16 commands
- **Processing**: First-come-first-served (fair arbitration)
- **Priority**: Emergency stop (0xFF) interrupts immediately
- Commands from both I2C and ESP-NOW are queued together

## Battery Management

- **Low Battery Threshold**: 3.4V or 20% charge
- **Behavior**: Restricts movement commands when battery is low
- **Monitoring**: Checks every 10 seconds
- **Status**: Reported via I2C status byte and serial output

## Servo Calibration

Trim values can be saved to non-volatile storage (Preferences):

```cpp
// Trim values stored as:
preferences.putInt("trim0", value); // -50 to +50 typical range
...
preferences.putInt("trim7", value);
```

**TODO**: Implement interactive calibration interface via serial

## Building and Flashing

```bash
# Using PlatformIO
pio run -t upload

# Monitor serial output
pio device monitor
```

## Example Usage

### Example Usage

```cpp
// From any ESP32 controller (Cardputer or Stack-Chan)
uint8_t command[2] = {0x02, 5}; // Walk forward 5 steps
esp_now_send(robotMAC, command, 2);
```

```cpp
// Turn left 3 steps
uint8_t command[2] = {0x04, 3};
esp_now_send(robotMAC, command, 2);
```

```cpp
// Jump
uint8_t command[2] = {0x08, 0};
esp_now_send(robotMAC, command, 2);
```

## Safety Features

1. **Emergency Stop**: Stops all movements immediately
2. **Low Battery Protection**: Restricts movement on low battery
3. **Command Queue Overflow**: Warns if queue is full
4. **Servo Limits**: PWM values clamped to safe range (150-600)

## TODO / Future Enhancements

- [ ] Interactive calibration interface via serial
- [ ] MAC address whitelist for ESP-NOW security
- [ ] EEPROM-based movement sequence storage
- [ ] Sensor integration (IMU for balance, ultrasonic for obstacle detection)
- [ ] Smooth movement interpolation between gaits
- [ ] Custom choreographed dance sequences
- [ ] OTA (Over-The-Air) firmware updates

## Credits

- Original miniKame design by Javier Isabel (JavierIH)
- Oscillator-based gait system inspired by Obijuan's work
- Hardware stack by Adafruit Industries

## License

This project inherits the Creative Commons BY-SA license from the original miniKame project.
