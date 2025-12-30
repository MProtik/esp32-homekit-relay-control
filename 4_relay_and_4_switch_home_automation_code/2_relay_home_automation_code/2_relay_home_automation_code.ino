#include "HomeSpan.h"
#include <Preferences.h>

Preferences prefs;

// ===== GPIO PINS =====
#define RELAY_1 2
#define RELAY_2 4

#define SWITCH_1 13
#define SWITCH_2 14

// ===== GLOBALS =====
unsigned long lastDebounce[2] = {0, 0};
bool lastState[2] = {HIGH, HIGH};
bool savedStates[2];
bool bootHandled[2] = {false, false};
unsigned long bootTime;

// ===== APPLIANCE CLASS =====
class Appliance : public Service::LightBulb {

  SpanCharacteristic *power;
  int relayPin;
  int switchPin;
  int index;

public:
  Appliance(int relay, int sw, int i) : Service::LightBulb() {
    relayPin = relay;
    switchPin = sw;
    index = i;

    pinMode(relayPin, OUTPUT);
    pinMode(switchPin, INPUT_PULLUP);

    power = new Characteristic::On();

    bool state = savedStates[i];
    power->setVal(state);
    digitalWrite(relayPin, state ? LOW : HIGH); // Active LOW relay
  }

  boolean update() override {
    bool newState = power->getNewVal();
    digitalWrite(relayPin, newState ? LOW : HIGH);

    prefs.begin("relayState", false);
    prefs.putBool(("r" + String(index)).c_str(), newState);
    prefs.end();

    return true;
  }

  void loop() override {
    bool current = digitalRead(switchPin);

    // Ignore switch during boot delay
    if (!bootHandled[index] && millis() - bootTime > 3000) {
      lastState[index] = current;
      bootHandled[index] = true;
      return;
    }

    // Handle manual switch
    if (bootHandled[index] &&
        current != lastState[index] &&
        millis() - lastDebounce[index] > 200) {

      bool newState = !power->getVal();
      power->setVal(newState);
      digitalWrite(relayPin, newState ? LOW : HIGH);

      prefs.begin("relayState", false);
      prefs.putBool(("r" + String(index)).c_str(), newState);
      prefs.end();

      lastDebounce[index] = millis();
    }

    lastState[index] = current;
  }
};

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  bootTime = millis();

  // Load saved relay states
  prefs.begin("relayState", true);
  savedStates[0] = prefs.getBool("r0", false);
  savedStates[1] = prefs.getBool("r1", false);
  prefs.end();

  homeSpan.setPairingCode("11112222");
  homeSpan.begin(Category::Lighting, "ESP32 2-Relay Board");

  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Identify();

  new Appliance(RELAY_1, SWITCH_1, 0);
  new Appliance(RELAY_2, SWITCH_2, 1);
}

// ===== LOOP =====
void loop() {
  homeSpan.poll();
}
