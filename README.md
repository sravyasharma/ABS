# 🌊 BMS – Borewell Management System (Smart Water Management System)

An **IoT-based Borewell Management System (BMS)** designed to automate borewell motor operations by monitoring water levels in real time. The system prevents water wastage, avoids dry-run damage to motors, and enables **remote monitoring and control using the Blynk platform**.

The system integrates **ultrasonic sensors, IoT connectivity, and relay-based automation** to ensure efficient and sustainable water usage.

---

# 📌 Problem Statement

Efficient water management is a major challenge in agriculture, households, and industries. Manual borewell operation often leads to:

- Water wastage  
- Motor dry runs  
- Excess energy consumption  
- Lack of monitoring  

This project leverages **IoT technology to automate water level monitoring and borewell pump control**, providing real-time insights and improving water management efficiency.

---

# 🎯 Project Objectives

- Automate borewell motor operation based on water level.
- Reduce human intervention in pump monitoring.
- Prevent motor damage caused by dry running.
- Provide real-time water level monitoring.
- Enable remote control and alerts using IoT.
- Improve energy efficiency and sustainability.

---

# 🚀 Key Features

### Water Level Monitoring
Uses an **ultrasonic sensor** to measure water level in the tank.

### Automatic Pump Control
Motor automatically:
- Turns **ON** when water level is low
- Turns **OFF** when tank is full

### IoT Dashboard
Real-time monitoring through the **Blynk IoT platform**.

### Manual & Automatic Modes
Users can switch between:
- Manual mode
- Auto mode

### Safety Protection
Includes protection mechanisms like:

- Dry-run prevention
- Overload protection
- Fault detection

### Data Monitoring
Displays water level data using **Blynk virtual pins and gauges**.

---

# 🧠 System Architecture

```
                +---------------------+
                |  Ultrasonic Sensor  |
                +----------+----------+
                           |
                           v
                +---------------------+
                |     ESP8266 / ESP32 |
                |  Microcontroller    |
                +----------+----------+
                           |
                           v
                +---------------------+
                |     Relay Module    |
                +----------+----------+
                           |
                           v
                +---------------------+
                |   Borewell Motor    |
                +----------+----------+
                           |
                           v
                +---------------------+
                |   Blynk Dashboard   |
                |  (Mobile Monitoring)|
                +---------------------+
```

---

# 🧩 System Modules

### 1️⃣ Water Level Monitoring Module
- Uses ultrasonic sensors to measure water levels.
- Provides real-time distance measurement.

### 2️⃣ Control Module
- Implemented using **ESP8266 / ESP32 microcontroller**.
- Controls relay switching for the borewell motor.

### 3️⃣ Communication Module
- WiFi communication using **Blynk IoT platform**.

### 4️⃣ Manual & Auto Mode Switching Module
- Allows switching between automatic and manual control.

### 5️⃣ Data Monitoring & Dashboard Module
- Displays water level percentage in the **Blynk console**.

### 6️⃣ Protection & Safety Module
- Dry run protection
- Motor safety mechanisms

---

# 🔧 Hardware Components

| Component | Description |
|---|---|
| ESP8266 / ESP32 | WiFi-enabled microcontroller |
| Ultrasonic Sensor (JSN-SR04T) | Measures water level |
| Relay Module | Controls motor switching |
| SX1278 LoRa Module | Wireless communication |
| 433MHz Antenna | Improves signal range |
| Push Buttons | Manual and mode switching |
| Jumper Wires | Circuit connections |
| Breadboard | Circuit prototyping |

Estimated Project Cost: **₹1,000 – ₹1,700**

---

# 🖥️ Software Stack

- Arduino IDE
- C++ (Embedded Programming)
- Blynk IoT Platform
- ESP8266WiFi Library
- NewPing Library

---

# ⚙️ Circuit Connections

### ESP8266 Connections

| Component | ESP8266 Pin |
|---|---|
| Ultrasonic TRIG | D1 |
| Ultrasonic ECHO | D2 |
| Relay Module | D5 |
| Manual Button | D6 |
| Mode Button | D0 |

---

### Power Connections

```
ESP8266 VCC  → 3.3V
ESP8266 GND  → Ground
```

---

### Ultrasonic Sensor

```
TRIG → D1
ECHO → D2
VCC  → 5V
GND  → Ground
```

---

### Relay Module

```
IN   → D5
VCC  → 3.3V
GND  → Ground
```

---

# 📱 Blynk Dashboard Configuration

Virtual Pins Used:

| Virtual Pin | Purpose |
|---|---|
| V1 | Relay control |
| V2 | Mode control |
| V3 | Relay status |
| V4 | Mode status |
| V5 | Water level gauge |

Steps:

1. Create an account on **Blynk Console**
2. Add device
3. Copy credentials:
   - Template ID
   - Template Name
   - Authentication Token
4. Update credentials in the code
5. Configure widgets like gauges and switches.

---

# 🔁 System Working Logic

## Automatic Mode

```
Water Level < 30%  → Pump ON
Water Level > 85%  → Pump OFF
```

The system continuously monitors the water level using the ultrasonic sensor and automatically controls the motor.

---

## Manual Mode

Users can manually control the pump using:

- Physical button
- Blynk mobile application

---

# 🌱 Advantages

- Water conservation
- Energy efficiency
- Reduced maintenance
- Remote monitoring
- Reliable pump operation
- Sustainable water management

---

# 📈 Future Improvements

- Mobile notification alerts
- Cloud-based analytics
- Solar-powered system
- AI-based water usage prediction
- Smart irrigation system integration

---

# 👨‍💻 Project Team

| Name | Roll No | Branch |
|---|---|---|
| M. Prathik Reddy | 23P81A6946 | CSO |
| Anumula Samarth | 23P81A6906 | CSO |
| Surya Kiran | 23P81A6952 | CSO |
| Ch. Sathvik Kumar | 23P81A6912 | CSO |
| D.L. Sravya | 23P81A6718 | CSD |

---

# 📜 License

This project is developed for **academic and research purposes**.
