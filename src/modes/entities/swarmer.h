#pragma once

#include "../../logic.h"
#include "../../stickfigure.h"

constexpr float gMinDistanceChase = 400.0f;
constexpr float gMinDistanceAttack = 40.0f;

class InGameState;
class Swarmer : public Entity {
public:
    Swarmer() = default;
    Swarmer(InGameState* game);

    int TypeId() const override { return TypeIdOf<Swarmer>(); }

    void OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) override;

    enum class State {
        IDLE,
        CHASING,
        ATTACK,
        HURT,
        DEAD,
        TO_IDLE
    };
    
    State state{ State::TO_IDLE };

    Figure figure;
    InGameState* game;
};
