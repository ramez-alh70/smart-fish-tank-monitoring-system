# 🐠 Smart Fish Tank Monitoring & Control System

An end-to-end **IoT Smart Aquarium Solution** featuring real-time water quality monitoring, automated fish feeding, relay pump control, and PWM lighting management via a web dashboard.

---

## 📌 System Overview

This project combines embedded systems engineering and modern web development into a unified architecture:
* **Arduino Uno:** Handles low-level sensor reading (Water Temperature, Turbidity, Ultrasonic Water Level) and hardware actuator control (Servo Feeder, Relay Pump, PWM LED).
* **ESP32:** Acts as an IoT Gateway & Web Server, serving a Web Dashboard and exposing RESTful API endpoints while communicating with the Arduino via UART.
* **Web Dashboard:** Interactive frontend (HTML/CSS/JS) to monitor real-time telemetry and control aquarium actuators remotely.

---

## 🚀 Key Features

* 🌡️ **Real-time Telemetry:** Live monitoring of Water Temperature (°C), Water Level (%), and Turbidity (%).
* 🐟 **Automated Fish Feeding:** Trigger servo motor feeding cycles remotely with a click.
* 🚰 **Smart Pump Control:** Remote relay switching for water circulation/filtration pump.
* 💡 **PWM LED Dimming:** Adjustable lighting control (0-100%) for natural day/night cycles.
* 🌐 **REST API & Web UI:** Fully featured RESTful HTTP endpoints with CORS support.

---

## 🛠️ Hardware & Tech Stack

### **Hardware Components**
* **Microcontrollers:** ESP32-WROOM-32, Arduino Uno
* **Sensors:** DS18B20 Water Temperature Sensor, HC-SR04 Ultrasonic Sensor, Analog Turbidity Sensor
* **Actuators:** SG90 Servo Motor, 5V Relay Module, PWM Dimmable LED Array
* **Communication:** UART Serial Communication (Serial2 on ESP32)

### **Software & Protocols**
* **Firmware:** C++ / Arduino Framework
* **Protocols:** HTTP REST API, UART Serial
* **Frontend:** HTML5, CSS3, JavaScript (Fetch API)

---

## 📂 Repository Structure

```text
smart-fish-tank-monitoring-system/
├── arduino_code/     # Sensor acquisition & actuator controller firmware
├── esp32_code/       # Web server, REST API & UART gateway firmware
├── web_dashboard/    # Frontend UI (HTML, CSS, JS)
├── assets/           # Project photos, schematics & demo media
└── README.md         # Project documentation

Method,Endpoint,Description
GET,/api/data,Fetch current sensor values & system status (JSON)
POST,/api/feed,Trigger fish feeder servo motor
POST,/api/pump,Toggle water pump state (ON/OFF)
POST,/api/led,Toggle LED lighting
POST,/api/brightness,Set LED brightness level (value=0-100)
