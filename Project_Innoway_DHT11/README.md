# ESP32 DHT11 MQTT (FreeRTOS) Project

An ESP32 (ESP-IDF) IoT demo using **FreeRTOS tasks** to read a **DHT11 temperature/humidity sensor**, connect to **Wi-Fi**, and communicate with a server via **MQTT**.  
The device publishes sensor data in **JSON** when a button is pressed, and listens for **warning messages** to blink LEDs.

---

## Features

- ✅ Read **temperature** and **humidity** from **DHT11** (GPIO 4)
- ✅ Connect to Wi-Fi (STA mode) using ESP-IDF networking stack
- ✅ MQTT publish/subscribe (JSON payloads)
- ✅ **FreeRTOS multitasking**:
  - Networking (MQTT) task
  - Button handling task
  - LED warning pattern task
- ✅ Warning indicators using two LEDs:
  - Temperature LED (GPIO 19)
  - Humidity LED (GPIO 5)

---

## Hardware Setup

| Component | GPIO | Notes |
|----------|------|------|
| DHT11 Data | GPIO 4 | Uses `dht11.h` / `DHT11_read()` |
| Button | GPIO 18 | Input with internal pull-up |
| LED (Temp / ND) | GPIO 19 | Output |
| LED (Humidity / DA) | GPIO 5 | Output |

> ND = “Nhiệt độ” (temperature)  
> DA = “Độ ẩm” (humidity)

---

## How It Works

### 1) Wi-Fi Connection
The device connects to Wi-Fi in station mode and waits until it gets an IP address using a **FreeRTOS Event Group**:
- `WIFI_CONNECTED_BIT`
- `WIFI_FAIL_BIT`

### 2) MQTT Communication
After Wi-Fi is connected, ESP32 connects to an MQTT broker and:
- **Publishes** data to `topicPub`
- **Subscribes** to warnings on `topicSub`

### 3) Button Actions (Publish Sensor Data)
The button selects what data to publish:

- **Short press**: toggles sending mode  
  - Temperature only: `{"ND":"<temp>"}`
  - Humidity only: `{"DA":"<hum>"}`

- **Long press (~2s)**: sends both  
  - `{"ND":"<temp>","DA":"<hum>"}`

### 4) Warning Messages (LED Patterns)
When the ESP32 receives MQTT data on the warning topic, it parses JSON and triggers LED patterns:

Examples:
- `{"warningND": "1"}` → blink Temperature LED
- `{"warningDA": "1"}` → blink Humidity LED
- `{"warningNDDA": "1"}` → turn BOTH LEDs ON, then OFF after ~5 seconds

---

## FreeRTOS Architecture (Tasks)

This project is built around **3 concurrent FreeRTOS tasks**:

### Task: `ConnectMQTT`
Responsibilities:
- Initialize MQTT client
- Publish sensor JSON when requested by button task
- Subscribe and handle incoming MQTT warning messages via callback

### Task: `Button`
Responsibilities:
- Read button GPIO
- Detect short vs long press (using `esp_timer_get_time()`)
- Set flags to trigger publishing in MQTT task

### Task: `StartLed`
Responsibilities:
- Control LED blinking patterns based on warning flags
- Uses timing + state machine logic to blink LEDs without blocking other tasks

---

## MQTT Topics

| Purpose | Topic |
|--------|-------|
| Publish sensor data (ESP → Server/Web) | `messages/90c8f03c-8cd1-4d59-8352-12a1e8d53ad8/topic` |
| Receive warnings (Server/Web → ESP) | `messages/90c8f03c-8cd1-4d59-8352-12a1e8d53ad8/topic/warning` |

---

## Build & Flash (ESP-IDF)

Make sure you have ESP-IDF installed and activated.

```bash
idf.py set-target esp32
idf.py menuconfig
idf.py build
idf.py flash monitor
