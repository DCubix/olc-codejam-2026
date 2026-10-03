#pragma once

#include <functional>

#include "../entities/player.h"

#include "../../logic.h"
#include "../../difficulty.hpp"
#include "../../tween.h"

constexpr float gDamageDisplayWidth = 80.0f;

class DamageDisplay : public HUDElement
{
public:
    void OnShow() override;
    void OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) override;

    void Bump();
    void Reset();

    Player* player;
    const DifficultyParams* params{&GetDifficulty(Difficulty::NORMAL)};

    std::function<void()> onExpire;

protected:
    float m_blinkTimer{0.0f};
    olc::vf2d m_textOff{0.0f, 0.0f};
    bool m_exiting{false};

    TweenAnimator m_tweens;
};
