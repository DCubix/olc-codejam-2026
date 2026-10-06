#pragma once

#include "../../stickfigure.h"
#include "../../logic.h"

class PlayerBullet : public Entity {
public:
    PlayerBullet() = default;
    PlayerBullet(const Sprite& spr);

    int TypeId() const override { return TypeIdOf<PlayerBullet>(); }

    void OnCreate(olc::PixelGameEngine* pge) override;
    void OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) override;
    void OnDraw(olc::PixelGameEngine* pge) override;

    void HitSomething();

    // set on the first hit, so one bullet cannot hit several enemies
    bool spent{false};

    olc::vf2d direction{};
    float speed{400.0f};
    int damage{10};

    bool gravity{false};
    float yVelocity{0.0f};
    int floorY{0};

    Figure figure;
};
