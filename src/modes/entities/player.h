#pragma once

#include "../../stickfigure.h"
#include "../../logic.h"

#include "../../repository.h"

struct Weapon {
    std::string name;
    std::string equipAssetPath, bulletAssetPath, soundAssetPath;

    float fireRate{0.2f};
    float projectileSpeed{400.0f};
    int damage{10};
    float spread{0.0f};
    int numProjectiles{1};
    bool gravity{false};
    float collisionRadius{0.0f};

    Sprite* equipAsset;
    Sprite* bulletAsset;
    ma::Sound* soundAsset;
};

inline Weapon gWeapons[] = {
    // Simple pistol
    { "Pistol", "gun1.png", "bullet1.png", "pistol.wav", 0.5f, 400.0f, 2, 0.0f, 1, false, 5.0f },
    // Shotgun
    { "Shotgun", "gun2.png", "bullet2.png", "shotgun.wav", 1.0f, 300.0f, 10, 20.0f, 4, false, 3.5f },
    // Machine-Gun
    { "Machine-Gun", "gun3.png", "bullet1.png", "machine-gun.wav", 0.25f, 600.0f, 5, 0.0f, 1, false, 5.0f },
    // Peas
    { "Peastol", "gun4.png", "bullet3.png", "pea.wav", 0.65f, 350.0f, 1, 5.0f, 1, true, 6.0f },
    // Ducks
    { "Quacknon", "gun5.png", "bullet4.png", "duck.wav", 1.1f, 400.0f, 1, 30.0f, 3, true, 14.0f },
    // Nerf Gun
    { "Nerf Gun", "gun6.png", "bullet5.png", "nerf.wav", 0.4f, 500.0f, 1, 1.0f, 1, false, 7.0f },
};

class InGameState;
class Player : public Entity {
public:
    Player() = default;
    Player(InGameState* game);

    int TypeId() const override { return TypeIdOf<Player>(); }

    void OnCreate(olc::PixelGameEngine* pge) override;
    void OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) override;
    void OnDraw(olc::PixelGameEngine* pge) override;

    void SwapWeapon();
    void TakeDamage(int value);

    bool IsDead() const { return health <= 0; }

    Figure figure;
    InGameState* game{nullptr};

    float shootTimer{0.1f};
    uint32_t weapon{0};

    int health{0}; // set from difficulty in the constructor
    int damageCounter{0};

    std::function<void()> onDeath;

private:
    bool m_flipX{false};
    float m_damageColorTimer{0.0f};
    float m_healthRechargeTimer{0.0f};
    float m_baseLeftArmAngle{0.0f};
};
