#include "WifiConnection.h"

#include <ESPmDNS.h>
#include <WiFi.h>

#include "secrets.h"

namespace {

bool hasWifiConfig() {
    return strlen(WIFI_SSID) > 0 && strlen(WIFI_PASSWORD) > 0;
}

}  // namespace

namespace WifiConnection {

bool connect() {
    if (!hasWifiConfig()) {
        Serial.println("WIFI_SSID/WIFI_PASSWORD fehlen. Lege include/secrets.h an.");
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(WIFI_HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.printf("Verbinde mit WLAN '%s'", WIFI_SSID);
    const unsigned long startMs = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startMs < 30000) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("Verbunden. IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    }

    Serial.println("WLAN-Verbindung fehlgeschlagen.");
    return false;
}

bool startMdnsHttpService(uint16_t port, const char *path) {
    if (WiFi.status() != WL_CONNECTED || !MDNS.begin(WIFI_HOSTNAME)) {
        return false;
    }

    MDNS.addService("http", "tcp", port);
    Serial.printf("mDNS aktiv: http://%s.local%s\n", WIFI_HOSTNAME, path);
    return true;
}

}  // namespace WifiConnection
