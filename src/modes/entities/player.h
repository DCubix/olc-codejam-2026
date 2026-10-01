#pragma once

#include "../../stickfigure.h"
#include "../../logic.h"

struct Weapon {
    std::string name;
    std::string equipAssetPath, bulletAssetPath;

    float fireRate{0.2f};
    float projectileSpeed{400.0f};
    int damage{10};
    float spread{0.0f};
    int numProjectiles{1};
    bool gravity{false};
    float collisionRadius{0.0f};

    Sprite* equipAsset;
    Sprite* bulletAsset;
};

inline Weapon gWeapons[] = {
    // Simple pistol
    { "Pistol", "assets/sprites/gun1.png", "assets/sprites/bullet1.png", 0.5f, 400.0f, 2, 0.0f, 1, false, 5.0f },
    // Shotgun
    { "Shotgun", "assets/sprites/gun2.png", "assets/sprites/bullet2.png", 1.0f, 300.0f, 10, 20.0f, 4, false, 3.5f },
    // Machine-Gun
    { "Machine-Gun", "assets/sprites/gun3.png", "assets/sprites/bullet1.png", 0.25f, 600.0f, 5, 0.0f, 1, false, 5.0f },
    // Peas
    { "Peastol", "assets/sprites/gun4.png", "assets/sprites/bullet3.png", 0.65f, 270.0f, 1, 5.0f, 1, true, 6.0f },
    // Ducks
    { "Quacknon", "assets/sprites/gun5.png", "assets/sprites/bullet4.png", 1.1f, 400.0f, 1, 30.0f, 3, true, 14.0f },
    // Nerf Gun
    { "Nerf Gun", "assets/sprites/gun6.png", "assets/sprites/bullet5.png", 0.4f, 500.0f, 1, 1.0f, 1, false, 7.0f },
};

class InGameState;
class Player : public Entity {
public:
    Player() = default;
    Player(InGameState* game);

    int TypeId() const override { return TypeIdOf<Player>(); }

    void OnCreate(olc::PixelGameEngine* pge) override;
    void OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) override;

    void SwapWeapon();

    Figure figure;
    InGameState* game;

    float shootTimer{0.1f};
    uint32_t weapon{4};
};
