#pragma once

#include "../../logic.h"
#include "../../stickfigure.h"

class InGameState;
class Swarmer : public Entity {
public:
    Swarmer() = default;
    Swarmer(InGameState* game);

    int TypeId() const override { return TypeIdOf<Swarmer>(); }

    void OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) override;
    void OnDraw(olc::PixelGameEngine* pge) override;

    enum class State {
        IDLE,
        CHASING,
        ATTACK,
        HURT,
        DEAD,
        TO_IDLE
    };
    
    State state{ State::TO_IDLE };
    bool m_hasHit{false};

    Figure figure;
    InGameState* game;

private:
    bool m_flipX{false};
};
