#pragma once

#include <cstdlib>
#include <numbers>

#include "olcPixelGameEngine3.h"

inline float Deg2Rad(float deg) {
    return deg * std::numbers::pi_v<float> / 180.0f;
}

inline float RandomF(float vmin, float vmax) {
    return vmin + (float(rand() % RAND_MAX) / float(RAND_MAX)) * (vmax - vmin);
}

inline olc::vf2d RotateVector(const olc::vf2d& v, float angleRad) {
    float c = std::cos(angleRad);
    float s = std::sin(angleRad);
    return olc::vf2d{
        c * v.x - s * v.y,
        s * v.x + c * v.y
    };
}
