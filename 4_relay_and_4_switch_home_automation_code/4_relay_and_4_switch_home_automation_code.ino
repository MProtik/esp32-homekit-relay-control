#include "HomeSpan.h"

// Assign GPIO pins for relays and switches
#define RELAY_1 2    // D2
#define RELAY_2 4    // D4
#define RELAY_3 18   // D18
#define RELAY_4 19   // D19

#define SWITCH_1 26  // D26
#define SWITCH_2 25  // D25
#define SWITCH_3 33  // D33
#define SWITCH_4 32  // D32

// Debounce helper
unsigned long lastDebounce[4] = {0, 0, 0, 0};
bool lastState[4] = {HIGH, HIGH, HIGH, HIGH};

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
    digitalWrite(relayPin, HIGH);  // Relay OFF initially for active-low logic

    pinMode(switchPin, INPUT_PULLUP);

    power = new Characteristic::On();
  }

  boolean update() override {
    // Invert logic for active-low relay
    digitalWrite(relayPin, power->getNewVal() ? LOW : HIGH);
    return true;
  }

  void loop() override {
    bool current = digitalRead(switchPin);

    // Toggle on any change (rocker switch)
    if (current != lastState[index] && millis() - lastDebounce[index] > 200) {
      bool newState = !power->getVal();                    // Toggle HomeKit state
      power->setVal(newState);                             // Update app
      digitalWrite(relayPin, newState ? LOW : HIGH);       // Relay logic inverted
      lastDebounce[index] = millis();
    }

    lastState[index] = current;
  }
};

void setup() {
  Serial.begin(115200);

  homeSpan.setPairingCode("11112222");
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
