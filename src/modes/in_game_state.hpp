#pragma once

#include <iostream>

#include "../tween.h"

#include "../logic.h"

#include "entities/player.h"
#include "entities/swarmer.h"
#include "../utils.hpp"
#include "hud/combodisplay.h"
#include "hud/weaponshuffler.h"

#include "utilities/olcUTIL3_Geometry2D.h"
namespace g2d = olc::utils::geom2d;


class InGameState : public GameMode {
public:
    InGameState() = default;
    ~InGameState() = default;

    InGameState(std::shared_ptr<GlobalGameData> pGlobalData) : GameMode(pGlobalData)
    {

    }

    GlobalGameData& GameData() { return *m_pGlobalData.get(); }

public:
    bool OnCreate(olc::PixelGameEngine* pge) override
    {   
        m_pGlobalData = std::make_shared<GlobalGameData>();

        auto player = Add<Player>(this);
        auto playerSize = player->figure.Size();
        player->position = pge->ScreenSize() / 2.0f + olc::vf2d{0.0f, playerSize.y/2.0f};

        comboDisplay.data = m_pGlobalData.get();
        comboDisplay.onExpire = [this]() {
            auto& gd = *m_pGlobalData;
            gd.hits = 0;
            gd.fireRateMultiplier = 0;
            gd.moveSpeedMultiplier = 0;
            gd.scoreMultiplier = 1;
            weaponShuffler.RequestStop();
        };

        weaponShuffler.onSwap = [this]() {
            if (auto p = Get<Player>()) {
                p->SwapWeapon();
                weaponShuffler.selectedWeapon = p->weapon;
            }
            return m_pGlobalData->combo > 0;
        };

        return false;
    }

    PlayState OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) override
    {
        // DEBUG
        if (pge->GetKeyboard().GetKey(olc::Key::K1).bPressed) { // simulate countdown at 5 seconds
            weaponShuffler.timer = 5.0f;
            weaponShuffler.Resume();
        }
        //

        int swarmers = 0;
        ForEachEntityOfType<Swarmer>([&](Swarmer& s) {
            swarmers++;
        });

        if (swarmers < 50) {
            timer += fElapsedTime;
            if (timer >= 0.3f) {
                timer = 0.0f;
                swarmers++;

                auto player = Get<Player>();
                auto playerSize = player->figure.Size();
                auto s = Add<Swarmer>(this);
                s->position = olc::vf2d{pge->ScreenSize().x / 2.0f + 300.0f, RandomF(80.0f, pge->ScreenSize().y-80.0f)};
            }
        }

        tweenAnimator.Update(fElapsedTime);

        auto& gd = *m_pGlobalData.get();

        auto& draw = pge->GetDraw();

        draw.Clear(olc::Pixel(0x44, 0x8E, 0xE4));

        std::stable_sort(entities.begin(), entities.end(), [](const auto& a, const auto& b) {
            return a->position.y < b->position.y;
        });

        draw.WorldOffset(cameraShaker);
        for (auto& e : entities) {
            e->Update(pge, fElapsedTime);
        }
        draw.WorldReset();

        for (auto& e1 : entities) {
            if (!e1->resolveCollision) continue;

            for (auto& e2 : entities) {
                if (e1 == e2) continue;

                if (!e2->resolveCollision) continue;

                g2d::circle<float> c1{e1->position, e1->colliderRadius};
                g2d::circle<float> c2{e2->position, e2->colliderRadius};

                if (!g2d::overlaps(c1, c2)) continue;

                auto diff = e1->position - e2->position;
                float dist = diff.mag();
                float overlap = (c1.radius + c2.radius) - dist;
                float pushRatio = e2->mass / (e1->mass + e2->mass);

                e1->position += (dist > 0.0f ? diff / dist : olc::vf2d{1.0f, 0.0f}) * overlap * pushRatio;
            }
        }

        // Add queued entities
        for (auto& e : entityQueue) {
            entities.push_back(std::move(e));
        }
        entityQueue.clear();

        // Remove dead entities
        entities.erase(std::remove_if(entities.begin(), entities.end(),
            [](const auto& e) { return e->IsDestroyed(); }), entities.end());

        comboDisplay.Update(pge, fElapsedTime);
        weaponShuffler.Update(pge, fElapsedTime);

        // show GD stats (debug)
        draw.StringProp(
            { 8, 8 },
            "Score: " + std::to_string(gd.score) + "\n"
            "Hits: " + std::to_string(gd.hits) + "\n"
            "Speed Mul.: " + std::to_string(gd.moveSpeedMultiplier) + "\n"
            "Fire Rate Mul.: " + std::to_string(gd.fireRateMultiplier) + "\n"
            "Score Mul.: " + std::to_string(gd.scoreMultiplier) + "\n"
            "Combo: " + std::to_string(gd.combo) + "\n"
            "Swarmers: " + std::to_string(swarmers) + "\n"
            "Swap Countdown: " + std::to_string(weaponShuffler.timer) + "\n",
            olc::Colour::BLACK
        );

        return PlayState::IN_GAME;
    }

    bool OnEnterMode(olc::PixelGameEngine* pge) override
    {
        return false;
    }

    bool OnExitMode(olc::PixelGameEngine* pge) override
    {
        return false;
    }

    template <typename T, typename... Args>
    T* Add(Args&&... args)
    {
        entityQueue.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        return dynamic_cast<T*>(entityQueue.back().get());
    }

    template <typename T>
    T* Get()
    {
        const int id = TypeIdOf<T>();
        for (auto& e : entities)
            if (e->TypeId() == id) return static_cast<T*>(e.get());
        return nullptr;
    }

    template <typename T>
    void ForEachEntityOfType(std::function<void(T&)> callback)
    {
        const int id = TypeIdOf<T>();
        for (auto& e : entities)
            if (e->TypeId() == id) callback(*static_cast<T*>(e.get()));
    }

    void AwardScore()
    {
        auto& gd = *m_pGlobalData.get();

        gd.score += 5 * gd.scoreMultiplier;
        gd.hits++;

        ShakeCamera(3.5f);

        if (gd.hits % gComboEveryNHits != 0) return;

        gd.combo++;

        if (gd.combo > 1) {
            comboDisplay.Bump();
            weaponShuffler.Resume();
            if (gd.combo == 2) weaponShuffler.ResetTimer();

            gd.fireRateMultiplier += gFireRateStepUpPerCombo;
            gd.moveSpeedMultiplier += gMoveSpeedMultiplierStepUpPerCombo;
            gd.scoreMultiplier += gScoreMultiplierStepUpPerCombo;

            gd.fireRateMultiplier = std::clamp(gd.fireRateMultiplier, 0, gMaxFireRateMultiplier);
            gd.moveSpeedMultiplier = std::clamp(gd.moveSpeedMultiplier, 0, 100);
            gd.scoreMultiplier = std::clamp(gd.scoreMultiplier, 1, 100);
        }
    }

    void ShakeCamera(float intensity)
    {
        const olc::vf2d v1 = olc::vf2d{
            RandomF(-1.0f, 1.0f), RandomF(-1.0f, 1.0f),
        }.norm() * intensity;
        const olc::vf2d v2 = olc::vf2d{
            RandomF(-1.0f, 1.0f), RandomF(-1.0f, 1.0f),
        }.norm() * intensity * 0.5f;
        const olc::vf2d v3 = olc::vf2d{
            RandomF(-1.0f, 1.0f), RandomF(-1.0f, 1.0f),
        }.norm() * intensity * 0.25f;
        tweenAnimator.Animate(&cameraShaker)
            .From(v1).To(v2).For(0.1f).Start();
        tweenAnimator.Animate(&cameraShaker)
            .Then().From(v2).To(v3).For(0.1f).Start();
        tweenAnimator.Animate(&cameraShaker)
            .Then().From(v3).To(olc::vf2d{}).For(0.1f).Start();
    }

    std::vector<std::unique_ptr<Entity>> entities;
    std::vector<std::unique_ptr<Entity>> entityQueue;

    float timer{0.0f};

    ComboDisplay comboDisplay;
    WeaponShuffler weaponShuffler;

    olc::vf2d cameraShaker{0.0f, 0.0f};

    TweenAnimator tweenAnimator;
};
