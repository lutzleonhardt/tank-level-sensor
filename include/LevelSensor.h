#pragma once

#include <Arduino.h>

namespace LevelSensor {

// Raw electrical reading from the ADS1115/shunt setup. This module deliberately
// stops at volts and milliamps; tank geometry belongs in TankLevelCalculator.
struct Reading {
    float rawAdcAverage;
    float voltageV;
    float currentMa;
    uint8_t sampleCount;
    uint8_t adcChannel;
    uint8_t adsAddress;
};

// Initializes I2C and the ADS1115. Returns false if the ADC is not reachable.
bool begin();
bool isReady();

// Reads and averages the configured ADS1115 channel, then converts the averaged
// shunt voltage to loop current. Returns false when the ADC failed to initialize.
bool read(Reading &reading);

}  // namespace LevelSensor
