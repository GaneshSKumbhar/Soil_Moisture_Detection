# SoilSense Pro 🚀

[![ESP32](https://img.shields.io/badge/ESP32-Dev%20Module-blue)](https://espressif.com)
[![Firebase](https://img.shields.io/badge/Firebase-Realtime%20DB-orange)](https://firebase.google.com)
[![Arduino](https://img.shields.io/badge/Arduino-IDE-green)](https://arduino.cc)

**Smart IoT Soil & Environmental Monitoring System**

**Complete Project Reference — Hardware + Software + Firebase**

**Board:** ESP32 Dev Module | **Firebase:** soilsense-iot-d1c31 | **By:** Ganesh Kumbhar

## 1. Project Overview

SoilSense Pro is a complete smart agriculture IoT system that monitors **soil moisture**, **temperature**, and **humidity** in real-time using an **ESP32 microcontroller**. Data is sent to **Firebase Realtime Database** every 5 seconds and displayed on a **live web dashboard** with:

- ✅ AI-based watering predictions
- ✅ Email alerts
- ✅ Hardware alerts (LED + Buzzer)

### Key Features

| Feature | Technology | Status |
|---------|------------|--------|
| Soil Moisture Monitoring | Capacitive Sensor + GPIO34 | ✅ Working |
| Temperature & Humidity | DHT11 + GPIO4 | ✅ Working |
| Cloud Database | Firebase Realtime DB | ✅ Working |
| Live Dashboard | HTML + Chart.js | ✅ Working |
| Login System | Firebase Authentication | ✅ Working |
| AI Watering Prediction | JavaScript Algorithm | ✅ Working |
| Email Alerts | EmailJS + Gmail | ✅ Working |
| LED Alert | External LED + GPIO26 | ✅ Working |
| Buzzer Alert | Active Buzzer + GPIO15 | ✅ Working |
| CSV Data Export | Browser Download | ✅ Working |
| Dark/Light Mode | CSS Theme Toggle | ✅ Working |

## 2. Components Required

| Component | Specification | Quantity |
|-----------|---------------|----------|
| ESP32 Dev Module | ESP32 WROOM-32, 38 pins | 1 |
| Soil Moisture Sensor | Capacitive type, 4-pin, 3.3V | 1 |
| DHT11 Sensor | Temperature + Humidity, 3-pin | 1 |
| LED | Any colour, 5mm external | 1 |
| Active Buzzer | 5V active buzzer, 2-pin | 1 |
| Resistor | 220 ohm (for LED) | 1 |
| Jumper Wires | Male-to-Male / any wire | 10+ |
| USB Cable | Micro USB for ESP32 | 1 |
| Power Supply | 5V USB adapter or laptop | 1 |

## 3. Complete Hardware Wiring

### 3.1 Soil Moisture Sensor (4-pin)
**Important:** Only 3 of 4 pins used. **DOUT pin unconnected.**

| Sensor Pin | Wire Colour | ESP32 Pin |
|------------|-------------|-----------|
| VCC (Power) | Red | 3V3 |
| GND (Ground) | Black | GND |
| AOUT (Analog) | Yellow | GPIO34 |
| DOUT (Digital) | — | **NOT CONNECTED** |

### 3.2 DHT11 Sensor (3-pin)
Pins ordered **left to right** (flat side facing you).

| DHT11 Pin | Wire Colour | ESP32 Pin |
|-----------|-------------|-----------|
| VCC (left) | Red | VIN (5V) |
| DATA (middle) | Green | GPIO4 |
| GND (right) | Black | GND |

### 3.3 LED (External)
**Longer leg = +**, shorter = -

| LED Leg | Wire Colour | ESP32 Pin |
|---------|-------------|-----------|
| + (long) | Red | GPIO26 (**via 220Ω resistor**) |
| - (short) | Black | GND |

**Note:** Direct GPIO26 to LED+ works without resistor (brighter).

### 3.4 Active Buzzer (2-pin)
**Longer leg/symbol = +**

| Buzzer Pin | Wire Colour | ESP32 Pin |
|------------|-------------|-----------|
| + (longer) | Red | GPIO15 |
| - (shorter) | Black | GND |

### 3.5 ESP32 Pin Summary

| ESP32 Pin | Connected To |
|-----------|--------------|
| **3V3** | Soil VCC (Red) |
| **VIN (5V)** | DHT11 VCC (Red) |
| **GND** | **All 4 black wires twisted together** |
| **GPIO34** | Soil AOUT (Yellow) |
| **GPIO4** | DHT11 DATA (Green) |
| **GPIO26** | LED+ (via resistor) |
| **GPIO15** | Buzzer+ (Red) |

**💡 TIP:** Use multiple GND pins or twist all grounds together.

## 4. Firebase Setup

1. **Create Project:** `console.firebase.google.com` → Add project → `soilsense-iot-d1c31`
2. **Realtime Database:** Build → Realtime DB → Create → **Test mode** → Enable
3. **Rules:** `.read` & `.write` → `true` → Publish
4. **Authentication:** 
   - Enable **Anonymous**
   - Enable **Email/Password**
5. **Web App:** Project Settings → `</>` → Copy `firebaseConfig`

**Project Details:**
```json
{
  "projectId": "soilsense-iot-d1c31",
  "databaseURL": "https://soilsense-iot-d1c31-default-rtdb.firebaseio.com/",
  "authDomain": "soilsense-iot-d1c31.firebaseapp.com"
}
```

**Database Path:** `/sensor/moisture/{timestamp}`

## 5. Arduino IDE Setup

1. **Board Manager:** Add URL `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
2. **Install:** ESP32 by Espressif
3. **Board:** `ESP32 Dev Module`
4. **Port:** COMx (e.g., COM8)
5. **Libraries:**
   ```
   Firebase Arduino Client (Mobizt)
   DHT sensor library (Adafruit)
   Adafruit Unified Sensor
   NTPClient (Fabrice Weinberg)
   ```

## 6. LED & Buzzer Alerts

| Condition | LED Action | Buzzer Action |
|-----------|------------|---------------|
| **Startup** | Blinks 3x | Beeps 2x |
| **Moisture OK (>30%)** | Quick blink/5s | Silent |
| **SOIL DRY (<30%)** | **SOLID ON** | — |
| **Dry alert/30s** | Blinks 3x → ON | Beeps 3x |
| **Recovered** | Blinks 2x → OFF | 1 long beep |

## 7. Dashboard Features
- **Login** → Email/Password
- **Live Gauge** → Real-time soil %
- **Charts** → Last 20 readings
- **Temp/Humidity Cards** → Min/Max/Avg
- **AI Prediction** → Next watering time
- **📧 Email Alerts** → Auto-send on dry
- **📊 CSV Export** → All data download
- **🌙 Dark/Light Mode**
- **Sign Out**

**Demo:** Open `soil-monitor-dashboard.html` or `soilsense-complete-guide.html` in browser.

**Full Firebase Config** (from soil-monitor-dashboard.html):
```javascript
const firebaseConfig = {
  apiKey: "AIzaSyDo4jyrbgxz7x585UEL9wKcfJQzWBwSPbM",
  authDomain: "soilsense-iot-d1c31.firebaseapp.com",
  databaseURL: "https://soilsense-iot-d1c31-default-rtdb.firebaseio.com",
  projectId: "soilsense-iot-d1c31",
  storageBucket: "soilsense-iot-d1c31.firebasestorage.app",
  messagingSenderId: "223496436985",
  appId: "1:223496436985:web:e284abbb8e1f5769a47628"
};
```

**EmailJS Config** (live alerts):
```
Service ID: service_rj3sq3y
Template ID: template_z90i26a  
Public Key: qPr-jzIwW1BHiFvsS
```

## 8. Firebase Data Structure

Path: `/sensor/moisture/{epoch}`

| Field | Description | Example |
|-------|-------------|---------|
| `value` | Soil % | 45.2 |
| `raw` | ADC (0-4095) | 1800 |
| `soil_status` | Dry/Moist/Wet | \"Moist\" |
| `temperature` | °C | 28.5 |
| `humidity` | % | 62 |
| `heat_index` | °C | 29.1 |
| `alert` | Dry alert | true/false |

## 9. EmailJS Setup

```
Service ID: service_rj3sq3y
Template ID: template_z90i26a
Public Key: qPr-jzIwW1BHiFvsS
Email: ganeshkumbhar638@gmail.com
```

**Trigger:** Soil <30% | **Cooldown:** 5min

## 10. Troubleshooting

| Problem | Cause | Fix |
|---------|-------|-----|
| **No LED** | Wrong wiring | GPIO26 → resistor → long leg → GND |
| **Buzzer always ON** | Wiring issue | Check GPIO15 |
| **Firebase fail** | Wrong URL | Remove `https://` prefix |
| **DHT11 NaN** | Loose wire | GPIO4 → VIN (not 3V3) |
| **Soil=0%** | Calibration | Adjust DRY/WET values |
| **No dashboard data** | Rules | `.read/.write = true` |
| **No emails** | Config | Check EmailJS keys |

## 11. Screenshots

### Dashboard Overview
![SoilSense Pro Dashboard](images/soilMoistureDashboard.png)
*Live gauges, AI predictions, temp/humidity cards, charts*

### Firebase Realtime Data
![Firebase RTDB](images/RealTimeDatabase.png)
*Live sensor/moisture/{timestamp} updates every 5s*

---

**SoilSense Pro** | **ESP32 IoT** | **Ganesh Kumbhar** | **soilsense-iot-d1c31**
