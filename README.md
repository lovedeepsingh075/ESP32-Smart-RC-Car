# ESP32 Smart RC Car using ESP-NOW (Automotive Lighting Automation)

## 📌 Project Overview
This project is an **ESP32-based Smart RC Vehicle** that implements realistic automotive control behavior.  
Unlike a basic RC car, this prototype includes **real vehicle lighting logic** such as brake lights, indicators, hazard mode, and headlights.

The system uses **ESP-NOW wireless communication** for low-latency real-time control between a **Transmitter ESP32** (remote controller) and a **Receiver ESP32** (car controller).

---

## 🎯 Key Features
- **ESP-NOW based low latency communication**
- **Servo-based steering control**
- **ESC throttle control**
- **Headlight ON/OFF toggle**
- **Hazard mode toggle**
- **Automatic brake light detection**
- **Automatic turn indicators**
- **Indicator memory** (keeps blinking after turning like a real car)
- **Steering center auto-calibration**
- **Trim calibration stored in EEPROM**
- **Failsafe system** (signal loss → stop motor + hazard blink)

---

## 🔧 Hardware Components Used

### Transmitter (Remote Unit)
- ESP32 Dev Board
- Analog joystick / potentiometer (Throttle + Steering)
- 4 Trim Buttons
- Headlight toggle button
- Hazard toggle button

### Receiver (Car Unit)
- ESP32 Dev Board
- Servo motor (Steering)
- ESC (Electronic Speed Controller)
- DC motor + gear system
- LEDs for:
  - Headlights
  - Front left indicator
  - Front right indicator
  - Rear left indicator
  - Rear right indicator
  - Brake light
- Battery pack / power supply

---

## 📡 Communication
- Protocol: **ESP-NOW**
- Type: **Peer-to-peer wireless control**
- Benefit: Low latency, stable control, no router required.

---

## 🧠 Working Logic Summary

### Throttle + Steering
- Joystick analog values are mapped into **0–255** range.
- Receiver maps them into **0–180 servo angle**.

### Steering Auto Center Learning
- When steering stays near neutral zone, receiver automatically learns center value.
- This helps maintain stable straight steering even after mechanical misalignment.

### Indicators & Hazard
- Steering direction automatically triggers left/right indicators.
- Indicators continue blinking for a short time even after steering returns to center.
- Hazard mode overrides all indicator logic.

### Brake Light Automation
- Brake light activates when throttle suddenly decreases (deceleration detection).
- Also activates when throttle is in low range (stop condition).

### Failsafe Mode
- If receiver does not receive any packet for a fixed timeout:
  - ESC stops (neutral)
  - Steering returns to center
  - Hazard lights blink
  - Brake light turns ON

---

## 📌 Pin Configuration

### Receiver ESP32
| Function | GPIO |
|---------|------|
| Steering Servo Signal | 25 |
| ESC Signal | 26 |
| Headlight | 4 |
| Front Left Indicator | 13 |
| Front Right Indicator | 15 |
| Rear Left Indicator | 12 |
| Rear Right Indicator | 14 |
| Brake Light | 27 |

### Transmitter ESP32
| Function | GPIO |
|---------|------|
| Throttle Analog | 32 |
| Steering Analog | 33 |
| Trim Throttle Up | 19 |
| Trim Throttle Down | 18 |
| Trim Steering Up | 23 |
| Trim Steering Down | 22 |
| Headlight Button | 4 |
| Hazard Button | 5 |

---

## 🗂️ Project Structure
---

## 🚀 How to Run the Project

### Step 1: Upload Transmitter Code
- Open `Transmitter/Transmitter.ino`
- Update receiver MAC address in code
- Upload to ESP32 transmitter board
### Step 2: Upload Receiver Code
- Open `Receiver/Receiver.ino`
- Upload to ESP32 receiver board

### Step 3: Test Features
- Move joystick to control throttle and steering
- Toggle headlights button
- Toggle hazard button
- Check brake light response
- Switch transmitter OFF to test failsafe mode

---

## 📌 Future Improvements
- Add wheel speed encoder for closed-loop speed control
- Add GPS tracking module
- Add mobile dashboard application
- Add obstacle avoidance sensors

---
## 🎥 Demo Video
Watch the full working demo here:  
https://youtu.be/Kl_gogzyiPI?si=lvmTH1cyn-LJM6R7

## 👤 Author
**Name:** (Lovedeep singh)  
**Country:** India  
**Domain:** Embedded Systems / IoT / Automotive Electronics  

---

## 📜 License
This project is for academic and learning purposes.
