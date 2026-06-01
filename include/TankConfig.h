#pragma once

#include <Arduino.h>

namespace TankConfig {

// ESP32 default I2C pins. The ADS1115 ADDR pin is tied to GND, so its address
// stays at 0x48.
constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint8_t ADS1115_ADDRESS = 0x48;
constexpr uint8_t LEVEL_ADC_CHANNEL = 0;
constexpr uint32_t I2C_CLOCK_HZ = 100000;

// Must match ads.setGain(GAIN_ONE). In this mode the ADS1115 maps signed raw
// counts to +/-4.096 V, i.e. one positive full-scale span is 32768 counts.
constexpr float ADS1115_GAIN_ONE_FULL_SCALE_V = 4.096f;
constexpr float ADS1115_COUNTS_PER_FULL_SCALE = 32768.0f;

// Tank level changes slowly, so averaging a short burst of synchronous ADS1115
// conversions reduces random ADC noise without hurting endpoint latency.
constexpr uint8_t ADC_SAMPLE_COUNT = 20;

// The hydrostatic probe outputs 4-20 mA for a 0-5 m water column. The ADS1115
// measures the voltage across this measured shunt resistor.
constexpr float SENSOR_MIN_CURRENT_MA = 4.0f;
constexpr float SENSOR_CURRENT_SPAN_MA = 16.0f;
constexpr float SENSOR_WATER_RANGE_M = 5.0f;
constexpr float LEVEL_SHUNT_RESISTOR_OHMS = 148.3f;

// Atlantis 7000: the normal overflow is 540 mm below the 2375 mm tank top.
// This makes the useful water column 1835 mm. The published usable volume is
// 6378 L; total geometric volume is 7000 L, but the area above the overflow is
// not usable storage.
constexpr float TANK_MAX_WATER_HEIGHT_M = 1.835f;
constexpr float TANK_USABLE_VOLUME_L = 6378.0f;

}  // namespace TankConfig
