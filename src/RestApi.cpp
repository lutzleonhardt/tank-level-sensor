#include "RestApi.h"

#include <Arduino.h>
#include <WebServer.h>

#include "LevelSensor.h"
#include "TankLevelCalculator.h"

namespace {

WebServer server(80);

void sendNoStore() {
    server.sendHeader("Cache-Control", "no-store");
}

void handleRoot() {
    server.send(200, "text/plain", "tank-level-sensor: GET /level");
}

void handleLevel() {
    LevelSensor::Reading sensorReading;
    if (!LevelSensor::read(sensorReading)) {
        sendNoStore();
        server.send(503, "application/json", "{\"status\":\"ads1115_unavailable\"}");
        return;
    }

    const TankLevel::Estimate estimate = TankLevel::calculate(sensorReading.currentMa);
    const int rawAdcRounded = static_cast<int>(sensorReading.rawAdcAverage + 0.5f);

    // Keep both raw electrical values and derived tank estimates in one response
    // so callers can debug calibration without serial access.
    char response[448];
    snprintf(response, sizeof(response),
             "{\"current_ma\":%.3f,\"voltage_v\":%.6f,\"raw_adc\":%d,"
             "\"raw_adc_avg\":%.1f,\"sample_count\":%u,"
             "\"level_m\":%.3f,\"height_percent\":%.1f,"
             "\"usable_volume_l_estimate\":%.0f,\"usable_volume_percent\":%.1f,"
             "\"adc_channel\":%u,\"ads_address\":\"0x%02X\",\"status\":\"ok\"}",
             sensorReading.currentMa, sensorReading.voltageV, rawAdcRounded,
             sensorReading.rawAdcAverage, sensorReading.sampleCount, estimate.levelM,
             estimate.heightPercent, estimate.usableVolumeL, estimate.usableVolumePercent,
             sensorReading.adcChannel, sensorReading.adsAddress);

    sendNoStore();
    server.send(200, "application/json", response);
}

void handleNotFound() {
    server.send(404, "application/json", "{\"status\":\"not_found\"}");
}

}  // namespace

namespace RestApi {

void begin() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/level", HTTP_GET, handleLevel);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP Server gestartet.");
}

void handleClient() {
    server.handleClient();
}

}  // namespace RestApi
