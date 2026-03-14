# Stack Kame Command Protocol Reference

> **Note**: Stack Kame uses ESP-NOW wireless protocol for all control commands.
> Both M5Stack Cardputer and Stack-Chan can control the robot wirelessly.
> Previous I2C slave mode has been removed to simplify bus architecture.

## Quick Command Reference

| Hex  | Dec | Name | Description | Value Range |
|------|-----|------|-------------|-------------|
| 0x01 | 1   | STOP | Stop and home | 0 |
| 0x02 | 2   | WALK_FWD | Walk forward | 1-255 steps |
| 0x03 | 3   | WALK_BACK | Walk backward | 1-255 steps |
| 0x04 | 4   | TURN_LEFT | Turn left | 1-255 steps |
| 0x05 | 5   | TURN_RIGHT | Turn right | 1-255 steps |
| 0x06 | 6   | MOON_FWD | Moonwalk forward | 1-255 steps |
| 0x07 | 7   | MOON_BACK | Moonwalk backward | 1-255 steps |
| 0x08 | 8   | JUMP | Jump in place | 0 |
| 0x09 | 9   | SHUFFLE_L | Shuffle left | 1-255 steps |
| 0x0A | 10  | SHUFFLE_R | Shuffle right | 1-255 steps |
| 0x0B | 11  | HOME | Home position | 0 |
| 0x0C | 12  | ZERO | All servos 90 degrees | 0 |
| 0xFF | 255 | E_STOP | Emergency stop | 0 |

## Movement Characteristics

### Walk (0x02, 0x03)
- **Speed**: ~900ms per step
- **Stability**: High
- **Power**: Medium
- **Best for**: General navigation

### Turn (0x04, 0x05)
- **Speed**: ~900ms per step
- **Rotation**: ~30° per step (approximate)
- **Stability**: High
- **Best for**: Direction changes

### Moonwalk (0x06, 0x07)
- **Speed**: ~1000ms per step
- **Stability**: Medium
- **Power**: High
- **Best for**: Show-off moves, smooth motion

### Shuffle (0x09, 0x0A)
- **Speed**: ~900ms per step
- **Stability**: Medium
- **Power**: Medium
- **Best for**: Side-to-side movement

### Jump (0x08)
- **Duration**: ~400ms
- **Stability**: Low (requires level surface)
- **Power**: High
- **Best for**: Celebration, attention-getting

## Status Monitoring

Status information is available via serial monitor output. The robot logs:
- Battery voltage and percentage every 10 seconds
- Command reception and execution
- Movement state changes
- Emergency stop status

Internal status tracking:
- Emergency stop active (triggered by command 0xFF)
- Low battery warning (below threshold)
- Command queue status (0-16 pending commands)

## Timing Guidelines

### Minimum Command Intervals
- Walk/Turn: 3-5 seconds for 3 steps
- Moonwalk: 4-6 seconds for 3 steps
- Shuffle: 3-5 seconds for 3 steps
- Jump: 0.5 seconds

### Queue Management
- Max queue size: 16 commands
- Processing: FIFO (first-come-first-served)
- Queue check: Poll status bit 2

### Battery Life
- Active movement: ~20-30 minutes (varies by battery size)
- Idle (servos at position): ~1-2 hours
- Low battery threshold: 3.4V or 20% charge

## Error Handling

### Robot Won't Move
1. Check emergency stop status (send 'r' via serial or clear e-stop)
2. Check battery level (read status byte bit 1)
3. Check queue full (read status byte bit 2)
4. Verify servo power supply

### Movement Unstable
1. Check servo calibration (adjust trim values)
2. Verify battery voltage >3.7V
3. Reduce step count or speed
4. Check for loose connections

### Command Not Executing
1. Verify I2C/ESP-NOW connection
2. Check command format (2 bytes required)
3. Verify robot not in emergency stop
4. Check serial monitor for error messages

## Example Command Sequences

### Square Pattern
```
Walk Forward (0x02, 5)
Wait 5s
Turn Left (0x04, 2)
Wait 2s
[Repeat 4 times]
```

### Dance Routine
```
Jump (0x08, 0)
Wait 0.5s
Moonwalk Forward (0x06, 2)
Wait 2s
Shuffle Left (0x09, 3)
Wait 3s
Shuffle Right (0x0A, 3)
Wait 3s
Turn Left (0x04, 4) // Spin!
```

### Exploration Pattern
```
Walk Forward (0x02, 10)
Wait 9s
Turn Right (0x05, 3) // Look around
Wait 3s
Walk Forward (0x02, 5)
Wait 5s
Turn Left (0x04, 6) // Head back
```

## Communication Examples

### ESP-NOW Command Format (2 bytes)
```
Data[0]: Command Type (0x01-0xFF)
Data[1]: Value (0-255)
```

**Example in Arduino (Cardputer/Stack-Chan):**
```cpp
uint8_t command[2];
command[0] = 0x02;  // Walk forward
command[1] = 5;     // 5 steps

esp_now_send(robotMAC, command, 2);
```

**Walk forward 3 steps:**
```
Byte 0: 0x02
Byte 1: 0x03
```

**Emergency stop:**
```
Byte 0: 0xFF
Byte 1: 0x00
```

## Debugging via Serial

Connect USB cable and use 115200 baud:

**Keyboard Commands:**
- `h` = Home position
- `z` = Zero servos
- `w` = Walk forward 3
- `b` = Walk backward 3
- `t` = Turn left
- `s` = Emergency stop
- `r` = Resume
- `q` = Show queue count

**Monitor Output:**
- Battery updates every 10s
- Command reception logs
- Movement execution status
- Error messages

## Protocol Extensions (Future)

Ideas for additional commands:
- `0x10-0x1F`: Custom dance sequences
- `0x20-0x2F`: Variable speed control
- `0x30-0x3F`: Sensor readings request
- `0x40-0x4F`: Servo calibration
- `0xF0-0xFE`: Configuration commands
