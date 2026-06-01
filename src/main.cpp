#include <Arduino.h>
#include "LevelSensor.h"
#include "RestApi.h"
#include "WifiConnection.h"

void setup() {
    Serial.begin(115200);
    delay(50);

    // Startup order keeps hardware, network, and HTTP concerns in their modules.
    LevelSensor::begin();
    const bool wifiConnected = WifiConnection::connect();
    RestApi::begin();

    if (wifiConnected) {
        WifiConnection::startMdnsHttpService(80, "/level");
    }
}

void loop() {
    RestApi::handleClient();
}
