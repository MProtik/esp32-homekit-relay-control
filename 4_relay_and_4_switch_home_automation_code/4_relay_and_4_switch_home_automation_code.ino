#include <WiFiManager.h>        // WiFiManager by tzapu
#include <WebServer.h>
#include "HomeSpan.h"

// Relay and switch pins
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

SpanCharacteristic* powerChar[4];
WebServer server(80);

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
    digitalWrite(relayPin, HIGH);

    pinMode(switchPin, INPUT_PULLUP);

    power = new Characteristic::On();
    powerChar[i] = power;
  }

  boolean update() override {
    digitalWrite(relayPin, power->getNewVal() ? LOW : HIGH);
    return true;
  }

  void loop() override {
    bool current = digitalRead(switchPin);
    if (current != lastState[index] && millis() - lastDebounce[index] > 200) {
      bool newState = !power->getVal();
      power->setVal(newState);
      digitalWrite(relayPin, newState ? LOW : HIGH);
      lastDebounce[index] = millis();
    }
    lastState[index] = current;
  }
};

void handleRelayControl(int index, bool state) {
  if (index < 0 || index > 3 || !powerChar[index]) {
    server.send(400, "text/plain", "Invalid relay");
    return;
  }

  powerChar[index]->setVal(state);
  digitalWrite((index == 0 ? RELAY_1 :
               index == 1 ? RELAY_2 :
               index == 2 ? RELAY_3 : RELAY_4), state ? LOW : HIGH);

  server.send(200, "text/plain", String("Relay ") + (index + 1) + (state ? " ON" : " OFF"));
}

void setupRoutes() {
  server.on("/relay/1/on", []() { handleRelayControl(0, true); });
  server.on("/relay/1/off", []() { handleRelayControl(0, false); });
  server.on("/relay/2/on", []() { handleRelayControl(1, true); });
  server.on("/relay/2/off", []() { handleRelayControl(1, false); });
  server.on("/relay/3/on", []() { handleRelayControl(2, true); });
  server.on("/relay/3/off", []() { handleRelayControl(2, false); });
  server.on("/relay/4/on", []() { handleRelayControl(3, true); });
  server.on("/relay/4/off", []() { handleRelayControl(3, false); });

  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found");
  });
}

void setup() {
  Serial.begin(115200);

  // WiFiManager for user-configurable WiFi setup
  WiFiManager wm;
  wm.setConfigPortalTimeout(180);  // Optional timeout
  if (!wm.autoConnect("ESP32-Setup", "12345678")) {
    Serial.println("Failed to connect. Restarting...");
    ESP.restart();
  }

  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  setupRoutes();

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
  server.handleClient();
  homeSpan.poll();
}
