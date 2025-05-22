#include "HomeSpan.h"
#include <Preferences.h>

Preferences prefs;

// GPIO pins
#define RELAY_1 2
#define RELAY_2 4
#define RELAY_3 18
#define RELAY_4 19

#define SWITCH_1 26
#define SWITCH_2 25
#define SWITCH_3 33
#define SWITCH_4 32

unsigned long lastDebounce[4] = {0, 0, 0, 0};
bool lastState[4] = {HIGH, HIGH, HIGH, HIGH};
bool savedStates[4];
bool bootHandled[4] = {false, false, false, false};  // Prevent toggle on first read

unsigned long bootTime;

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
    digitalWrite(relayPin, state ? LOW : HIGH);
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

    // During boot delay, do NOT toggle — just update baseline state
    if (!bootHandled[index] && millis() - bootTime > 3000) {
      lastState[index] = current;         // Set clean baseline after boot delay
      bootHandled[index] = true;          // Don't do this again
      return;
    }

    // After bootHandled is true, handle switch normally
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

void setup() {
  Serial.begin(115200);
  bootTime = millis();

  // Load saved states
  prefs.begin("relayState", true);
  savedStates[0] = prefs.getBool("r0", false);
  savedStates[1] = prefs.getBool("r1", false);
  savedStates[2] = prefs.getBool("r2", false);
  savedStates[3] = prefs.getBool("r3", false);
  prefs.end();

  homeSpan.setPairingCode("********");
  homeSpan.begin(Category::Lighting, "ESP32 Relay Board");

  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Identify();

  new Appliance(RELAY_1, SWITCH_1, 0);
  new Appliance(RELAY_2, SWITCH_2, 1);
  new Appliance(RELAY_3, SWITCH_3, 2);
  new Appliance(RELAY_4, SWITCH_4, 3);

  Serial.println("Use this HomeKit code in Apple Home: 111-22-333");
}

void loop() {
  homeSpan.poll();
}
