#include <Arduino.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>
#include "secrets.h"

WebServer server(80);

bool hasWifiConfig() {
    return strlen(WIFI_SSID) > 0 && strlen(WIFI_PASSWORD) > 0;
}

void handleLevel() {
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json",
                "{\"current_ma\":14.1,\"raw_adc\":712,\"status\":\"ok\"}");
}

void handleRoot() {
    server.send(200, "text/plain", "tank-level-sensor: GET /level");
}

void handleNotFound() {
    server.send(404, "application/json", "{\"status\":\"not_found\"}");
}

void connectWifi() {
    if (!hasWifiConfig()) {
        Serial.println("WIFI_SSID/WIFI_PASSWORD fehlen. Lege include/secrets.h an.");
        return;
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
    } else {
        Serial.println("WLAN-Verbindung fehlgeschlagen.");
    }
}

void setup() {
    Serial.begin(115200);
    delay(50);

    connectWifi();

    server.on("/", HTTP_GET, handleRoot);
    server.on("/level", HTTP_GET, handleLevel);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP Server gestartet.");

    if (WiFi.status() == WL_CONNECTED && MDNS.begin(WIFI_HOSTNAME)) {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("mDNS aktiv: http://%s.local/level\n", WIFI_HOSTNAME);
    }
}

void loop() {
    server.handleClient();
}
