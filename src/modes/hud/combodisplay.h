#pragma once

#include <functional>

#include "../../logic.h"
#include "../../tween.h"

constexpr float gComboResetTimer = 2.5f;

class ComboDisplay : public HUDElement {
public:
    void OnShow() override;
    void OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) override;

    // Call when the combo count goes up. Restarts the expiry timer.
    void Bump();

    GlobalGameData* data{nullptr};
    // Called once when the timer runs out, before the exit animation.
    std::function<void()> onExpire;

protected:
    float m_timer{0.0f};
    olc::vf2d m_textOff{0.0f, 0.0f};
    bool m_exiting{false};

    TweenAnimator m_tweens;
};
