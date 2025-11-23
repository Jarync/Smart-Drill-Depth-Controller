# Smart Drill Depth Control System (Auto-Stop)

![Platform](https://img.shields.io/badge/Platform-Arduino%20%7C%20ESP8266-blue.svg)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20%28Embedded%29-orange.svg)
![Protocol](https://img.shields.io/badge/Communication-ESP--NOW%20%7C%20UART-green.svg)

## 📖 Introduction

This project is an embedded control system designed to automate drilling operations by introducing a precision **auto-stop mechanism**. It solves the problem of over-drilling by continuously monitoring the depth in real-time and wirelessly cutting off power to the drill once a pre-set target depth is reached.

The system is split into two modules:
1.  **Measurement Unit (Handheld):** Mounted on the drill, measuring distance and handling user input.
2.  **Power Control Unit (Remote):** Connected to the main power supply, responsible for switching the relay.

## 🛠️ Bill of Materials (BOM)

Based on the firmware logic and system design, the following hardware components were used to build the prototype:

### 1. Measurement Unit (Drill Attachment)
* **Microcontroller:** Arduino Nano (V3.0)
    * *Role: Central processing, state machine management, sensor data acquisition.*
* **Wireless Module:** ESP8266 (e.g., ESP-01 or NodeMCU)
    * *Role: Wi-Fi Client (Sender), communicates with Nano via Serial (UART).*
* **Distance Sensor:** VL53L0X Time-of-Flight (ToF) Laser Sensor
    * *Role: Millimeter-level precision distance measurement.*
* **Display:** 0.96" OLED Display (I2C, SSD1306 Driver)
    * *Role: UI for setting depth, deadzone, and viewing status.*
* **Input:** 3x Push Buttons (with Pull-up resistors/internal pull-up)
    * *Role: Menu navigation (+, -, Confirm).*
* **Power Supply:** 9V Battery or Li-Po Battery (for Nano/ESP).

### 2. Power Control Unit (Wall Plug / Socket)
* **Microcontroller:** ESP8266 (NodeMCU or ESP-12F)
    * *Role: Wi-Fi Slave (Receiver).*
* **Actuator:** 5V/12V Relay Module (High Voltage Control)
    * *Role: Physically cuts the AC power to the drill.*
* **Input:** 1x Reset Button
    * *Role: System reset/Manual override.*
* **Power Supply:** 5V AC-DC Adapter (to power the ESP).

---

## ⚙️ System Architecture & Logic

The system operates using a low-latency wireless link to ensure safety and precision.

<img width="8330" height="7689" alt="drilling_bb" src="https://github.com/user-attachments/assets/6bd3d99d-4ede-4300-a95e-ed1f47df22d3" />


### 1. Architecture Diagram
```text
[ Drill / Handheld Unit ]                       [ Power Socket Unit ]
|                                               |
|  [VL53L0X] --(I2C)--> [Arduino Nano]          |
|                           |                   |
|  [OLED UI] <--(I2C)-------+                   |
|                           |                   |
|  [Buttons] --(GPIO)-------+                   |
|                           |                   |
|                        (Serial/UART)          |
|                           |                   |
|                       [ESP8266 Client]  >>> (ESP-NOW Wireless) >>>  [ESP8266 Slave] ----(GPIO)----> [Relay]
|                                                                           |
|                                                                      [AC Power Cut]
```
---

## 📸 Prototype Gallery

The following images demonstrate the functional prototype, including the wiring setup and the OLED interface in action.

### 1. Hardware Setup
<img width="448" height="290" alt="屏幕截图 2025-11-23 211538" src="https://github.com/user-attachments/assets/6be1b1d1-cf80-4cfa-ae89-f52f227dc8d6" />

### 2. User Interface (OLED)
<img width="278" height="224" alt="屏幕截图 2025-11-23 211800" src="https://github.com/user-attachments/assets/36c11262-b515-4230-b8e5-b49409603fee" />

