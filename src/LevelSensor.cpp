#include "LevelSensor.h"

#include <Adafruit_ADS1X15.h>
#include <Wire.h>

#include "TankConfig.h"

namespace {

Adafruit_ADS1115 ads;
bool adsReady = false;

}  // namespace

namespace LevelSensor {

bool begin() {
    Wire.begin(TankConfig::I2C_SDA_PIN, TankConfig::I2C_SCL_PIN);
    Wire.setClock(TankConfig::I2C_CLOCK_HZ);

    ads.setGain(GAIN_ONE);
    adsReady = ads.begin(TankConfig::ADS1115_ADDRESS, &Wire);

    if (adsReady) {
        Serial.printf("ADS1115 bereit: Adresse 0x%02X, SDA GPIO%u, SCL GPIO%u\n",
                      TankConfig::ADS1115_ADDRESS, TankConfig::I2C_SDA_PIN,
                      TankConfig::I2C_SCL_PIN);
    } else {
        Serial.printf("ADS1115 nicht gefunden: Adresse 0x%02X pruefen.\n",
                      TankConfig::ADS1115_ADDRESS);
    }

    return adsReady;
}

bool isReady() {
    return adsReady;
}

bool read(Reading &reading) {
    if (!adsReady) {
        return false;
    }

    int32_t rawAdcSum = 0;
    for (uint8_t i = 0; i < TankConfig::ADC_SAMPLE_COUNT; i++) {
        rawAdcSum += ads.readADC_SingleEnded(TankConfig::LEVEL_ADC_CHANNEL);
    }

    const float rawAdcAverage =
        static_cast<float>(rawAdcSum) / static_cast<float>(TankConfig::ADC_SAMPLE_COUNT);
    const float voltage = rawAdcAverage * TankConfig::ADS1115_GAIN_ONE_FULL_SCALE_V /
                          TankConfig::ADS1115_COUNTS_PER_FULL_SCALE;

    // Ohm's law: the loop current is the ADS1115 voltage across the measured
    // 148.3 ohm shunt. Multiplying by 1000 converts A to mA.
    reading.rawAdcAverage = rawAdcAverage;
    reading.voltageV = voltage;
    reading.currentMa = voltage / TankConfig::LEVEL_SHUNT_RESISTOR_OHMS * 1000.0f;
    reading.sampleCount = TankConfig::ADC_SAMPLE_COUNT;
    reading.adcChannel = TankConfig::LEVEL_ADC_CHANNEL;
    reading.adsAddress = TankConfig::ADS1115_ADDRESS;

    return true;
}

}  // namespace LevelSensor
