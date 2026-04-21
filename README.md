# 🗑️ Smart Recycling Bin

> An Arduino-based automated waste segregation system with real-time fill monitoring, touchless lid control, and wet/dry waste sorting — built for homes, classrooms, and public spaces.

[![Platform](https://img.shields.io/badge/Platform-Arduino%20Uno-00979D?logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Language](https://img.shields.io/badge/Language-C%2B%2B%20%2F%20Arduino-blue)](https://www.arduino.cc/reference/en/)
[![License](https://img.shields.io/badge/License-MIT-green)](LICENSE)
[![Status](https://img.shields.io/badge/Status-Active-brightgreen)]()

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [How It Works](#how-it-works)
- [Hardware Requirements](#hardware-requirements)
- [Circuit & Pin Mapping](#circuit--pin-mapping)
- [Software & Libraries](#software--libraries)
- [Installation & Upload](#installation--upload)
- [Configuration](#configuration)
- [LCD Display Guide](#lcd-display-guide)
- [Serial Monitor Output](#serial-monitor-output)
- [Troubleshooting](#troubleshooting)
- [Project Structure](#project-structure)
- [Future Improvements](#future-improvements)
- [Contributing](#contributing)
- [License](#license)
- [Authors](#authors)

---

## Overview

The **Smart Recycling Bin** is an embedded system project that automates the most friction-heavy part of waste management — sorting. Instead of relying on the user to decide where to throw something, this system uses an IR sensor and a moisture sensor to classify incoming waste as **dry** or **wet**, then routes it to the correct compartment using a servo-driven sorting flap.

A second set of ultrasonic sensors monitors how full each compartment is. When either compartment approaches capacity, the lid locks and a buzzer alerts the user. Fill levels are displayed in real time on a 16×2 I2C LCD.

The entire interaction is **touchless** — the lid opens automatically when a hand is detected within 15 cm of the top sensor, stays open for 5 seconds, then closes on its own.

---

## Features

| Feature | Description |
|---|---|
| **Touchless lid** | HC-SR04 detects hand proximity; lid opens and closes automatically |
| **Waste classification** | IR + moisture sensors classify waste as dry or wet in real time |
| **Auto-sorting flap** | Servo-driven flap routes waste to the correct compartment |
| **Dual fill monitoring** | Separate ultrasonic sensors track dry and wet compartment fill % |
| **Full-bin lockout** | Lid locks and buzzer alerts when any compartment exceeds 90% |
| **LCD status display** | 16×2 I2C LCD shows fill levels and current system state |
| **Serial logging** | All events printed to Serial Monitor for debugging |
| **Boot sequence** | Startup beep and welcome message confirm system is alive |

---

## How It Works

```
Hand detected (< 15 cm)
        │
        ▼
  Lid servo opens  ──── Bin full? ──── YES ──▶ Lock lid + Buzzer alert
        │
      (1 s delay — waste settles)
        │
        ▼
  IR sensor + Moisture sensor read simultaneously
        │
   ┌────┴────┐
   │         │
  DRY       WET
   │         │
Flap →30°  Flap →150°
(dry side) (wet side)
        │
        ▼
  Flap returns to 90° (neutral)
        │
        ▼
  After 5 s → Lid closes automatically
        │
        ▼
  LCD updates fill % continuously
```

### Sensor Logic Details

**Waste type detection** runs 1 second after the lid opens (gives time for the item to land on the sensor zone):

```
IR  = LOW  &&  Moisture = HIGH  →  DRY waste  (flap → 30°)
IR  = LOW  &&  Moisture = LOW   →  WET waste  (flap → 150°)
IR  = HIGH (no object)          →  No action, lid times out normally
```

**Fill level calculation** converts ultrasonic distance to a percentage:

```
BIN_HEIGHT = 30 cm  (full travel of sensor)
EMPTY at  ~28 cm  →  0%
FULL  at   ~5 cm  →  100%

fill% = (EMPTY_DIST - dist) / (EMPTY_DIST - FULL_DIST) × 100
```

---

## Hardware Requirements

| Component | Quantity | Notes |
|---|---|---|
| Arduino Uno (or Nano) | 1 | Main microcontroller |
| HC-SR04 Ultrasonic Sensor | 3 | Lid, dry bin, wet bin |
| IR Obstacle Sensor Module | 1 | Waste presence detection |
| Soil / Moisture Sensor | 1 | Wet waste classification |
| SG90 Servo Motor | 2 | Lid servo + sorting flap servo |
| Passive Buzzer | 1 | Audio alerts |
| 16×2 I2C LCD (PCF8574) | 1 | Status display (address 0x27) |
| 5V Power Supply / USB | 1 | Minimum 1A recommended with servos |
| Jumper Wires | — | Male-to-male and male-to-female |
| Breadboard or PCB | 1 | For prototyping |
| Physical bin enclosure | 1 | DIY or repurposed dual-compartment bin |

> ⚠️ **Power note:** Two servo motors drawing current simultaneously can brown-out an Arduino powered from USB alone. Use a separate 5V 1A supply for the servos if you see resets or erratic behaviour.

---

## Circuit & Pin Mapping

```
Arduino Pin   Connected To          Direction
───────────   ─────────────────     ─────────
D2            HC-SR04 Lid TRIG      OUTPUT
D3            HC-SR04 Lid ECHO      INPUT
D4            IR Sensor OUT         INPUT
D5            Moisture Sensor OUT   INPUT
D6            Lid Servo Signal      OUTPUT (PWM)
D7            Flap Servo Signal     OUTPUT (PWM)
D8            HC-SR04 Dry TRIG      OUTPUT
D9            HC-SR04 Dry ECHO      INPUT
A0            HC-SR04 Wet TRIG      OUTPUT
A1            HC-SR04 Wet ECHO      INPUT
D12           Passive Buzzer        OUTPUT
A4 (SDA)      LCD SDA               I2C
A5 (SCL)      LCD SCL               I2C
5V            VCC (all modules)     POWER
GND           GND (all modules)     GROUND
```

> **I2C address:** The LCD defaults to `0x27`. If it doesn't display anything, try `0x3F`. You can scan for the correct address using an [I2C scanner sketch](https://playground.arduino.cc/Main/I2cScanner/).

---

## Software & Libraries

### Required Libraries

Install these via **Arduino IDE → Sketch → Include Library → Manage Libraries**:

| Library | Version | Purpose |
|---|---|---|
| `LiquidCrystal_I2C` | ≥ 1.1.2 | I2C LCD control |
| `Servo` | Built-in | Servo motor control |
| `Wire` | Built-in | I2C communication |

### IDE

- Arduino IDE **1.8.x** or **2.x**
- Board: **Arduino Uno** (or compatible)
- Programmer: **AVRISP mkII** / standard USB

---

## Installation & Upload

1. **Clone this repository**
   ```bash
   git clone https://github.com/your-username/smart-recycling-bin.git
   cd smart-recycling-bin
   ```

2. **Open the sketch**
   ```
   Arduino IDE → File → Open → SmartBin.ino
   ```

3. **Install libraries** (if not already installed)
   ```
   Sketch → Include Library → Manage Libraries
   Search: "LiquidCrystal I2C" → Install
   ```

4. **Select board and port**
   ```
   Tools → Board → Arduino Uno
   Tools → Port → COMx (Windows) or /dev/ttyUSBx (Linux/Mac)
   ```

5. **Upload**
   ```
   Sketch → Upload  (or Ctrl+U)
   ```

6. **Verify**
   - LCD should show `Smart Recycle / Bin ON :)` for 2 seconds
   - Two beeps confirm system ready
   - Open Serial Monitor at **9600 baud** to see live logs

---

## Configuration

All tuneable parameters are at the top of `SmartBin.ino` as named constants. **You should not need to touch anything else.**

```cpp
// Physical bin height (sensor to bin bottom, in cm)
const float BIN_HEIGHT = 30.0;

// Distance threshold to trigger lid open (cm)
const float LID_TRIGGER = 15.0;

// How long lid stays open after no waste is detected (ms)
const unsigned long LID_DURATION = 5000;

// Fill % at which bin is considered full and lid locks
const int FULL_THRESHOLD = 90;
```

### Servo System

```cpp
lidServo.write(0);    // Lid CLOSED position
lidServo.write(90);   // Lid OPEN position

flapServo.write(90);  // Flap NEUTRAL (centre)
flapServo.write(30);  // Flap → DRY compartment
flapServo.write(150); // Flap → WET compartment
```

Adjust these angles to match your physical build. Servos vary — test with `servo.write()` in isolation first.

---

## LCD Display Guide

| Row | Content | Example |
|---|---|---|
| Row 0 | Fill levels (dry % / wet %) | `Dry:45% Wet:30% ` |
| Row 1 | Current system state | `Lid Open...` |

**Row 1 states:**

```
"Smart Recycle"   → Boot message
"Lid Open..."     → Hand detected, lid opening
">> DRY Waste"    → Dry waste detected and sorted
">> WET Waste"    → Wet waste detected and sorted
"Lid Closed"      → Lid closed after timeout
"BIN FULL! LOCKED"→ Compartment ≥ 90%, lid locked
```

---

## Serial Monitor Output

Open Serial Monitor at **9600 baud** to see a live event log:

```
SYSTEM READY
LID OPEN
SORTED: DRY
LID CLOSED
LID OPEN
SORTED: WET
LID CLOSED
BIN FULL — LID LOCKED
```

This is especially useful during calibration and debugging.

---

## Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| LCD shows nothing | Wrong I2C address | Run I2C scanner; try `0x3F` instead of `0x27` |
| LCD shows blocks only | Contrast too low | Adjust the blue potentiometer on back of LCD |
| Lid doesn't open | Hand too far, or ECHO wiring | Check pin D2/D3, test sensor with `measureDistance()` alone |
| Servo jitters on power-up | Insufficient current | Use external 5V 1A supply for servos |
| Buzzer makes no sound | Wrong pin or passive buzzer | Confirm `tone()` works with passive; active buzzers need `digitalWrite` |
| Moisture sensor wrong | Sensor needs calibration | Adjust threshold: change `== LOW` to `< analogRead threshold` if using analog mode |
| Wrong fill % reading | `BIN_HEIGHT` mismatch | Measure actual bin height and update the constant |
| Both waste types misclassified | IR/moisture swapped in wiring | Swap D4 and D5 connections or swap in `#define` |

---

## Project Structure

```
smart-recycling-bin/
│
├── SmartBin.ino          ← Main Arduino sketch
├── README.md             ← This file
├── LICENSE               ← MIT License
│
├── docs/
│   ├── circuit_diagram.png    ← Wiring diagram (add your own)
│   └── project_report.pdf     ← Full project report (optional)
│
└── images/
    ├── bin_front.jpg          ← Photo of finished build
    └── lcd_display.jpg        ← LCD in action
```

---

## Future Improvements

- [ ] **GSM/Wi-Fi alert** — SMS or push notification when bin is full (SIM800L or ESP8266)
- [ ] **IoT dashboard** — Send fill level data to ThingSpeak or Blynk for remote monitoring
- [ ] **Weight sensor** — Load cell (HX711) for more accurate fill estimation
- [ ] **Colour-based sorting** — TCS3200 colour sensor to sort recyclables (paper, plastic, metal)
- [ ] **Solar charging** — Small solar panel + LiPo for off-grid operation
- [ ] **Compaction motor** — DC motor to compress dry waste when bin is 70%+ full
- [ ] **OTA updates** — Swap Arduino for ESP32 to enable over-the-air firmware updates
- [ ] **Data logging** — SD card module to log timestamps of each disposal event

---

## Contributing

Contributions are welcome! Here's how:

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-feature-name`
3. Commit your changes: `git commit -m "Add: your feature description"`
4. Push to your fork: `git push origin feature/your-feature-name`
5. Open a Pull Request against `main`

Please keep code changes consistent with the existing style — named constants, descriptive function names, and a comment on any non-obvious logic.

---

## License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

You are free to use, modify, and distribute this project for personal, educational, or commercial purposes with attribution.

---

## Authors

**Sailesh Saini** — [GitHub](https://github.com/your-username)  
D. Y. Patil Institute of MCA and Management, Pune

**Snehal Khode** — Contributor  

---

> *"The best waste management system is the one people actually use. Make it effortless."*

---

<p align="center">Made with ☕ and solder | Arduino • C++ • Sensors</p>
