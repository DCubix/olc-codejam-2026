#pragma once

#include <functional>

#include "../../logic.h"
#include "../../difficulty.hpp"
#include "../../tween.h"
#include "../../utils.hpp"

// Counts down to a weapon swap and shows the shuffle frame near the end.
// Visible = countdown running.
class WeaponShuffler : public HUDElement {
public:
    void OnShow() override;
    void OnHide() override;
    void OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) override;

    // Start (or keep) the countdown.
    void Resume() { m_stopping = false; Show(); }
    // Stop the countdown. Hides immediately unless a weapon was already chosen (HOLD/HIDE), which stays on screen until its out animation ends.
    void RequestStop();

    void ResetTimer() { timer = RandomF(params->swapCountdownMin, params->swapCountdownMax); }

    const DifficultyParams* params{&GetDifficulty(Difficulty::NORMAL)};

    float timer{0.0f};
    // Weapon shown during HOLD. Set by onSwap.
    uint32_t selectedWeapon{0};
    // Called when the countdown ends. Return true to keep running (timer is re-randomized); false stops after the selected weapon has been shown.
    std::function<bool()> onSwap;

protected:
    enum class State {
        IDLE,
        SHOW,
        SHUFFLE,
        HOLD,
        HIDE,
    };

    void SetState(State s);

    State m_state{State::IDLE};
    float m_stateTime{0.0f};
    bool m_stopping{false};

    float m_frameSize{0.0f};
    const float m_maxFrameSize{68.0f};
    const float m_minFrameSize{2.0f};
    const float m_slideTime{0.5f};

    TweenAnimator m_tweens;

    int m_lastWeapon{0};
};
