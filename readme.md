# BLE Keyboard Web Controller

## Overview
This ESP32-based project creates a web-controlled BLE (Bluetooth Low Energy) keyboard that allows you to:
- Type text in a web interface and send it to any BLE-enabled device
- View connection status through an LED indicator
- Cancel ongoing transmissions (Not working)
- Monitor system status via serial output

## Features
- **Web Interface**: Responsive HTML/CSS interface accessible from any device
- **BLE Keyboard Emulation**: Acts as a standard Bluetooth keyboard
- **Visual Feedback**: Built-in LED shows connection status:
  - Solid blue: Both WiFi and BLE connected
  - Blinking blue: Only BLE connected
  - Off: Not connected
- **Real-time Control**: Cancel button for interrupting long transmissions
- **Status Monitoring**: Serial console output for debugging

## Hardware Requirements
- ESP32 development board
- Micro USB cable for power/programming

## Software Dependencies
- Arduino IDE with ESP32 core
- Required libraries:
  - `WiFi`
  - `WebServer`
  - `BleKeyboard` (install via Library Manager)

## Setup Instructions

1. **Install Dependencies**
   - Install the ESP32 board package in Arduino IDE
   - Install the required libraries

2. **Configuration**
   - Create a `config.h` file with your WiFi credentials:
     ```cpp
     const char* ssid = "YOUR_WIFI_SSID";
     const char* password = "YOUR_WIFI_PASSWORD";
     ```

3. **Upload the Sketch**
   - Connect your ESP32 via USB
   - Select the correct board and port in Arduino IDE
   - Upload the sketch

4. **Connect to the Web Interface**
   - After upload, check the serial monitor for the IP address
   - Open a web browser and navigate to the displayed IP

## Usage
1. **Web Interface Controls**:
   - **Text Area**: Type or paste text to send
   - **Send Text**: Transmits the text to connected BLE device
   - **Clear Text**: Clears the text area
   - **Cancel Send**: Stops an ongoing transmission

2. **LED Indicators**:
   - Monitor connection status without needing the serial monitor

3. **Serial Monitor** (115200 baud):
   - View detailed connection status
   - Debug information
   - Transmission progress

## Technical Details
- **Web Server**: Runs on port 80
- **BLE Keyboard**: Emulates standard HID keyboard
- **Character Transmission**: 20ms delay between characters for reliability
- **Memory Usage**: HTML/CSS stored in PROGMEM to conserve RAM

## Troubleshooting
- **Connection Issues**:
  - Check serial monitor for error messages
  - Ensure BLE device is in pairing mode
  - Verify WiFi credentials in `config.h`

- **Performance Problems**:
  - Reduce text chunk size if experiencing lag
  - Ensure ESP32 has good power supply

