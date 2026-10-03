#pragma once

#include "../../logic.h"
#include "../../difficulty.hpp"
#include "../entities/player.h"

class StatsDisplay : public HUDElement {
public:
    StatsDisplay();

    void OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) override;

    Player* player{nullptr};
    GlobalGameData* data{nullptr};
    const DifficultyParams* params{&GetDifficulty(Difficulty::NORMAL)};

    float scoreTimer{0.0f};
};
