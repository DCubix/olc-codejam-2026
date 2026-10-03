#pragma once

#include <functional>

#include "../../logic.h"
#include "../../difficulty.hpp"
#include "../../tween.h"

class ComboDisplay : public HUDElement {
public:
    void OnShow() override;
    void OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) override;

    // Call when the combo count goes up. Restarts the expiry timer.
    void Bump();

    // Called when the players takes damage, or when the timer runs out.
    void Reset();

    // True while a combo of 2 or more is shown and not already ending.
    bool Active() const { return m_visible && !m_exiting; }

    GlobalGameData* data{nullptr};
    const DifficultyParams* params{&GetDifficulty(Difficulty::NORMAL)};
    std::function<void()> onExpire;

protected:
    float m_timer{0.0f};
    olc::vf2d m_textOff{0.0f, 0.0f};
    bool m_exiting{false};

    TweenAnimator m_tweens;
};
