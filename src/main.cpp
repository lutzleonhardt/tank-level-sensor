#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>
#include "secrets.h"

constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint8_t ADS1115_ADDRESS = 0x48;
constexpr uint8_t LEVEL_ADC_CHANNEL = 0;
constexpr uint32_t I2C_CLOCK_HZ = 100000;

constexpr float LEVEL_SHUNT_RESISTOR_OHMS = 148.3f;

Adafruit_ADS1115 ads;
WebServer server(80);
bool adsReady = false;

bool hasWifiConfig() {
    return strlen(WIFI_SSID) > 0 && strlen(WIFI_PASSWORD) > 0;
}

void handleLevel() {
    if (!adsReady) {
        server.sendHeader("Cache-Control", "no-store");
        server.send(503, "application/json",
                    "{\"status\":\"ads1115_unavailable\"}");
        return;
    }

    const int16_t rawAdc = ads.readADC_SingleEnded(LEVEL_ADC_CHANNEL);
    const float voltage = ads.computeVolts(rawAdc);
    const float currentMa = voltage / LEVEL_SHUNT_RESISTOR_OHMS * 1000.0f;

    char response[192];
    snprintf(response, sizeof(response),
             "{\"current_ma\":%.3f,\"voltage_v\":%.6f,\"raw_adc\":%d,"
             "\"adc_channel\":%u,\"ads_address\":\"0x%02X\",\"status\":\"ok\"}",
             currentMa, voltage, rawAdc, LEVEL_ADC_CHANNEL, ADS1115_ADDRESS);

    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", response);
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

void setupAds1115() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(I2C_CLOCK_HZ);
    ads.setGain(GAIN_ONE);
    adsReady = ads.begin(ADS1115_ADDRESS, &Wire);

    if (adsReady) {
        Serial.printf("ADS1115 bereit: Adresse 0x%02X, SDA GPIO%u, SCL GPIO%u\n",
                      ADS1115_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN);
    } else {
        Serial.printf("ADS1115 nicht gefunden: Adresse 0x%02X pruefen.\n",
                      ADS1115_ADDRESS);
    }
}

void setup() {
    Serial.begin(115200);
    delay(50);

    setupAds1115();
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
