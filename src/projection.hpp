#pragma once

#include "olcPixelGameEngine3.h"

constexpr float gDefaultScale = 4.0f;

inline olc::vf2d WorldToScreen(float x, float y, float z, olc::vf2d size, float scale = gDefaultScale) {
    const float c = std::cos(45.0f);
    float sy = y * c - z * c;
    return olc::vf2d{ size.x / 2.0f + x * scale, size.y / 2.0f + sy * scale };
}

inline olc::vf2d ScreenToWorld(olc::vf2d screenPos, olc::vf2d size, float z = 0.0f, float scale = gDefaultScale) {
    const float c = std::cos(45.0f);

    float sx = (screenPos.x - size.x / 2.0f) / scale;
    float sy = (screenPos.y - size.y / 2.0f) / scale;

    float x = sx;
    float y = (sy + z * c) / c;

    return olc::vf2d{ x, y };
}
