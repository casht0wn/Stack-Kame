# Stack Kame Controller

*for* **M5Stack Cardputer-ADV**

<img src="https://shop.m5stack.com/cdn/shop/files/9_e979ae23-d47f-4037-9da6-5c3bc25e0362_1200x1200.webp" alt="Cardputer-ADV" height="160"/>

This is currently a work in progress! ⚠️

The purpose of this firmware is to be able to control a Stack-Kame robot [^1] using an M5Stack Cardputer-ADV. It will be a simple controller with basic movement control as well as calibration and diagnostic information. 

[^1]: Basically a Stack-Chan on top of a mini-Kame quadruped.


## UI Navigation

### Controls
| Key | Function |
|-----|----------|
| ⬆️up | move up |
| ⬇️down | move down |
| ⬅️left | move left |
| ➡️right | move right |
| ok/enter | select |
| bksp/del | go back |

### Screens
- Main Menu 
    - Main Controls (`MainControlScreen`)
        - Manual Control of Stack Kame via ESP-NOW commands.
    - Diagnostics (`DiagnosticsScreen`)
        - View Diagnostic info for Stack Kame and the Cardputer such as MAC addresses, battery levels, and RSSI. 
    - Calibration (`CalibrationScreen`)
        - Calibrate servo direction, trims, and end points to ensure Stack Kame walks as desired. 

