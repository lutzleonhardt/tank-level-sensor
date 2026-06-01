#include "TankLevelCalculator.h"

#include <Arduino.h>

#include "TankConfig.h"

namespace {

struct VolumeCalibrationPoint {
    float heightM;
    float volumeL;
};

// These Atlantis 7000 points are derived from manufacturer retention variants:
// water height = 2375 mm tank height - published outlet depth. The associated
// volume is the manufacturer's usable volume below that outlet. Linear
// interpolation between points is an estimate, but it follows the real tank
// shape much better than treating height and liters as directly proportional.
constexpr VolumeCalibrationPoint TANK_VOLUME_CURVE[] = {
    {0.0f, 0.0f},
    {0.708f, 2450.0f},
    {0.874f, 3200.0f},
    {1.168f, 4550.0f},
    {TankConfig::TANK_MAX_WATER_HEIGHT_M, TankConfig::TANK_USABLE_VOLUME_L},
};

float clampFloat(float value, float minValue, float maxValue) {
    if (value < minValue) {
        return minValue;
    }
    if (value > maxValue) {
        return maxValue;
    }
    return value;
}

float calculateLevelMeters(float currentMa) {
    const float sensorFraction =
        clampFloat((currentMa - TankConfig::SENSOR_MIN_CURRENT_MA) /
                       TankConfig::SENSOR_CURRENT_SPAN_MA,
                   0.0f, 1.0f);
    return sensorFraction * TankConfig::SENSOR_WATER_RANGE_M;
}

float estimateUsableVolumeLiters(float levelM) {
    constexpr size_t pointCount = sizeof(TANK_VOLUME_CURVE) / sizeof(TANK_VOLUME_CURVE[0]);
    const float clampedLevel = clampFloat(levelM, TANK_VOLUME_CURVE[0].heightM,
                                          TANK_VOLUME_CURVE[pointCount - 1].heightM);

    for (size_t i = 1; i < pointCount; i++) {
        const VolumeCalibrationPoint lower = TANK_VOLUME_CURVE[i - 1];
        const VolumeCalibrationPoint upper = TANK_VOLUME_CURVE[i];

        if (clampedLevel <= upper.heightM) {
            const float segmentFraction =
                (clampedLevel - lower.heightM) / (upper.heightM - lower.heightM);
            return lower.volumeL + segmentFraction * (upper.volumeL - lower.volumeL);
        }
    }

    return TankConfig::TANK_USABLE_VOLUME_L;
}

}  // namespace

namespace TankLevel {

Estimate calculate(float currentMa) {
    const float levelM = calculateLevelMeters(currentMa);
    const float heightPercent =
        clampFloat(levelM / TankConfig::TANK_MAX_WATER_HEIGHT_M * 100.0f, 0.0f, 100.0f);
    const float usableVolumeL = estimateUsableVolumeLiters(levelM);
    const float usableVolumePercent = clampFloat(
        usableVolumeL / TankConfig::TANK_USABLE_VOLUME_L * 100.0f, 0.0f, 100.0f);

    return {levelM, heightPercent, usableVolumeL, usableVolumePercent};
}

}  // namespace TankLevel
