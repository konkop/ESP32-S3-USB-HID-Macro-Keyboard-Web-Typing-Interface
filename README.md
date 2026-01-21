# ESP32-S3 USB HID Macro Keyboard + Web Typing Interface

This project turns an ESP32-S3 into a USB HID keyboard with a built-in web interface that can type text into another computer in a human-like way and trigger macro commands using physical Cherry MX keys.

---

## Features

- USB HID Keyboard (TinyUSB)
- Web interface hosted on the ESP32
- Press Enter to send text (Shift+Enter inserts newline)
- Human-like typing (random delays, pauses, backspaces)
- 9 physical Cherry MX macro keys
- Debounced button handling
- Serial debug output
- mDNS support (.local address)
- PlatformIO compatible

---

## Hardware Required

### Main Board
- ESP32-S3 development board with native USB
- Tested with: ESP32-S3 DevKit (N16R8)
  - https://www.amazon.com/dp/B0CKXJLP4B?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1

### Buttons
- 9x Cherry MX switches
- Optional hot-swap sockets
- Optional keycaps

### Wiring
- Jumper wires
- USB-C data cables (must support data, not power-only)

---

## GPIO Pin Mapping (Physical Keys)

Each Cherry MX switch connects between a GPIO pin and GND.
Internal pull-ups are used (INPUT_PULLUP).

| Button | GPIO | Action |
|------|------|--------|
| 1 | GPIO 4 | Ctrl + C |
| 2 | GPIO 5 | Ctrl + V |
| 3 | GPIO 12 | Ctrl + A |
| 4 | GPIO 13 | KEEP ALIVE -> www.google.com (win only) |
| 5 | GPIO 14 | Alt + Tab |
| 6 | GPIO 15 | Ctrl + X |
| 7 | GPIO 16 | Ctrl + Alt + Del (delay) enter |
| 8 | GPIO 17 | Ctrl + S |
| 9 | GPIO 18 | Ctrl + P |

### Wiring Diagram (Concept)

GPIO ----[ Cherry MX Switch ]---- GND

- Button pressed = LOW
- Button released = HIGH
- No external resistors required

---

## Software Setup (PlatformIO)

### Install Tools
- Visual Studio Code
- PlatformIO extension

### Clone Repository

git clone https://github.com/YOUR_USERNAME/esp32-s3-hid-typing-keyboard.git
cd esp32-s3-hid-typing-keyboard

### platformio.ini

[env:esp32-s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino

monitor_speed = 115200

build_flags =
  -D ARDUINO_USB_MODE=1
  -D ARDUINO_USB_CDC_ON_BOOT=0

---

## Wi-Fi Configuration

Edit main.cpp and set your Wi-Fi credentials:

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

---

## Uploading and Running

### USB Connections

- UART / COM port: flashing firmware and Serial Monitor
- Native USB port: HID keyboard device

Both ports can be connected to the same computer at the same time.

### Upload Firmware

Use PlatformIO "Upload".

### Serial Monitor

Use PlatformIO "Monitor".

Expected output:

WiFi Connected
IP Address: xxx.xxx.xxx.xxx
http://human-typing-keyboard.local

---

## Web Interface

Open the following in your browser:

http://human-typing-keyboard.local

- Press Enter to send text
- Shift + Enter inserts a newline
- Page does not reload after sending

---

## Physical Macro Keys

- Each key triggers once per press
- Fully debounced in software
- Serial output confirms key presses

Example:

Button 1 pressed: Ctrl+C

---

## Debugging Notes

- Buttons are active LOW
- HIGH = unpressed
- LOW = pressed
- Use Serial output to verify wiring

---

## Notes

- ESP32-S3 supports 2.4 GHz Wi-Fi only
- Many USB-C cables are power-only — use data-capable cables
- HID typing controls your computer — use with caution
- This is built as a demonstration use at your own risk. I take no responsiblity for actions with this device.

---

## Future Improvements

- Web-configurable macros
- Key remapping
- OLED display support

---

## License

MIT License

---

## Credits

- ESP32 Arduino Core
- TinyUSB
- PlatformIO
