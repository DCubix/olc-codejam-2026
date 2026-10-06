#pragma once

#include "utilities/olcUTIL3_GameMode.h"

constexpr float gHUDPadding = 40.0f;

int NextTypeId();

template<class T>
int TypeIdOf() { static const int id = NextTypeId(); return id; }

class Entity {
public:
    Entity() = default;
    virtual ~Entity() = default;

    virtual int TypeId() const = 0;

    virtual void OnCreate(olc::PixelGameEngine* pge) {}
    virtual void OnDestroy(olc::PixelGameEngine* pge) {}
    virtual void OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) {}
    virtual void OnDraw(olc::PixelGameEngine* pge) {}

    void Update(olc::PixelGameEngine* pge, float fElapsedTime);
    void Draw(olc::PixelGameEngine* pge);

    void Destroy(float timeout = 0.0f);
    bool IsDestroyed() const { return m_destroyed; }

    olc::vf2d position;
    bool resolveCollision{true};
    float colliderRadius{8.0f};
    float mass{1.0f};

protected:
    float m_destroyTimeout{-1.0f};
    bool m_destroyed{false}, m_initialized{false};
};

// QUIT is after MAX_STATES: it has no mode, main.cpp ends the app when it is returned
enum class PlayState { MENU = 0, ABOUT, IN_GAME, MAX_STATES, QUIT };
struct GlobalGameData {
    int score{0};
    int combo{0}, hits{0};
    int scoreMultiplier{1};
    int fireRateMultiplier{0};
    int moveSpeedMultiplier{0};
};

using GameMode = olc::utils::gsm::Mode<GlobalGameData, PlayState>;

class HUDElement {
public:
    HUDElement() = default;
    ~HUDElement() = default;

    virtual void OnShow() {}
    virtual void OnHide() {}
    virtual void OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) {}
    virtual void OnDraw(olc::PixelGameEngine* pge) {}

    void Update(olc::PixelGameEngine* pge, float fElapsedTime);
    void Draw(olc::PixelGameEngine* pge);

    void Show();
    void Hide();
    bool IsVisible() const { return m_visible; }
protected:
    bool m_visible{false};
};
