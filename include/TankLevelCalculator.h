#pragma once

namespace TankLevel {

// Derived tank state. Height is a direct pressure-sensor result; volume is an
// Atlantis-7000-specific estimate from manufacturer calibration points.
struct Estimate {
    float levelM;
    float heightPercent;
    float usableVolumeL;
    float usableVolumePercent;
};

// Converts 4-20 mA probe current into water column height and estimated usable
// tank volume.
Estimate calculate(float currentMa);

}  // namespace TankLevel
