#pragma once

#include <Arduino.h>

namespace WifiConnection {

// Connects to the configured station network from include/secrets.h.
bool connect();

// Starts tank-level-sensor.local advertisement once WiFi is connected.
bool startMdnsHttpService(uint16_t port, const char *path);

}  // namespace WifiConnection
