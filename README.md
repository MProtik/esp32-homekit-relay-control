# ESP32 HomeKit Relay Control

This project enables control of up to **4 relays** using an **ESP32** board, with support for both:
- **Apple HomeKit integration** via [HomeSpan](https://github.com/HomeSpan/HomeSpan)
- **Manual toggle switches** (rocker style) connected to the ESP32

Each relay can be used to switch lights, fans (on/off), or other appliances. Manual switches provide local control even without a phone.

---

## 🔌 Features

- ✅ Control via Apple Home app
- ✅ Control via physical rocker switches
- ✅ Real-time state synchronization between HomeKit and physical switch
- ✅ Active-low relay logic (LOW = ON)
- ✅ Debounce logic to prevent switch bouncing
- ✅ Works with standard 4-channel relay modules (active-low)

---

## 🛠️ Hardware Requirements

- ESP32 development board (e.g., DOIT ESP32 DevKit V1)
- 4-channel 5V or 12V **active-low** relay module
- AC appliances (bulbs/fans etc.)
- Rocker/toggle switches x 4
- Power supply (depending on relay voltage)

### 🪛 Relay Wiring Notes

- Relay INx pins → ESP32 GPIOs
- Relay VCC → 5V / 12V (based on module)
- Relay GND → ESP32 GND
- Appliances are connected via relay NO (Normally Open) and COM (Common) terminals

---

## 🧠 Software Stack

- **Platform:** Arduino + ESP32
- **HomeKit Library:** [HomeSpan](https://github.com/HomeSpan/HomeSpan)
- **Language:** C++

---

## 🧩 Pin Configuration

| Relay # | Relay GPIO | Switch GPIO |
|---------|------------|-------------|
| Relay 1 | GPIO 2     | GPIO 26     |
| Relay 2 | GPIO 4     | GPIO 25     |
| Relay 3 | GPIO 18    | GPIO 33     |
| Relay 4 | GPIO 19    | GPIO 32     |

> All switches use `INPUT_PULLUP` configuration.

---

## 🔧 Installation

1. Install [Arduino IDE](https://www.arduino.cc/en/software) and [ESP32 Board Package](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
2. Install [HomeSpan](https://github.com/HomeSpan/HomeSpan) via Library Manager
3. Open `esp32-homekit-relay-control.ino`
4. Upload the code to your ESP32
5. Add the device in your iPhone using the pairing code `111-22-333`

---

## 🏠 Apple Home Setup

1. Open **Apple Home** app
2. Tap **+ Add Accessory**
3. Enter manual code: `111-22-333`
4. Assign each relay to a room or name it (e.g., Living Room Light)

---

## 📷 Demo (optional)

> Add wiring photos or a short video of the ESP32 and relays in action

---

## 📜 License

This project is open-source and released under the [MIT License](LICENSE).

---

## 🙋‍♂️ Support / Contributions

Feel free to open issues or submit PRs for improvements. Suggestions welcome!


