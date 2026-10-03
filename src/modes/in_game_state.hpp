#pragma once

#include <iostream>

#include "../tween.h"

#include "../logic.h"

#include "entities/player.h"
#include "entities/swarmer.h"
#include "entities/tank.h"
#include "../utils.hpp"
#include "hud/combodisplay.h"
#include "hud/weaponshuffler.h"
#include "hud/damagedisplay.h"

#include "utilities/olcUTIL3_Geometry2D.h"
namespace g2d = olc::utils::geom2d;

constexpr float kFar = 0.7f;      // tilt: horizontal scale of the top screen row (bottom row is 1)
constexpr float kAmbient = 0.05f; // brightness outside the circle
constexpr int kTile = 32;
constexpr olc::vi2d kFloorTiles{48, 32};
constexpr olc::vf2d kFloorMax{kFloorTiles.x * kTile / 2.0f, kFloorTiles.y * kTile / 2.0f};
constexpr olc::vf2d kFloorMin{-kFloorMax.x, -kFloorMax.y};

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

        damageDisplay.player = player;

        // Lights
        const float lightMargin = 2.0f * kTile;
        for (int i = 0; i < 18; i++) {
            Light l;
            l.pos = olc::vf2d{
                RandomF(kFloorMin.x + lightMargin, kFloorMax.x - lightMargin),
                RandomF(kFloorMin.y + lightMargin, kFloorMax.y - lightMargin)
            };
            l.radius = RandomF(80.0f, 300.0f);
            lights.push_back(l);
        }

        /**
uniform vec2 pgeTargetSizeInPixels;			// Size of the target olc::Image in pixels
uniform vec2 pgeInverseTargetSizeInPixels;  // 1.0 / Size of the target olc::Image in pixels
uniform float pgeTotalTimeElapsed;			// Total time elapsed since application started
uniform sampler2D pgeTexture0;				// Current source olc::Image bound as texture0
uniform sampler2D pgeTexture1;				// Current source olc::Image bound as texture1
uniform sampler2D pgeTexture2;				// Current source olc::Image bound as texture2
uniform sampler2D pgeTexture3;				// Current source olc::Image bound as texture3

// Inputs from Vertex Shader
in vec2 oTex;
in vec4 oCol;
         **/

        // FX
        fxRedVignetteShader.SetVertexShaderSource(
            olc::gpu::Shader_GLSL33::VS_DefaultHeader() +
            olc::gpu::Shader_GLSL33::VS_DefaultMain()
        );
        fxRedVignetteShader.SetPixelShaderSource(
            olc::gpu::Shader_GLSL33::PS_DefaultHeader() + R"(
            uniform float uIntensity;
            void main() {
                vec2 uv = gl_FragCoord.xy * pgeInverseTargetSizeInPixels;
                uv *= 1.0 - uv.yx;

                float vig = uv.x * uv.y * 15.0;
                vig = 1.0 - pow(vig, clamp(uIntensity, 0.0, 1.0));

                pixel = vec4(vig, 0.0, 0.0, 1.0);
            })"
        );
        if (fxRedVignetteShader.Compile() != "OK") return false;
        fxRedVignetteShader.CreateUniform("uIntensity");

        pge->CreateImage(fxRedVignetteTex, pge->ScreenSize());

        return false;
    }

    PlayState OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) override
    {
        // Add queued entities
        for (auto& e : entityQueue) {
            entities.push_back(std::move(e));
        }
        entityQueue.clear();


        auto player = Get<Player>();

        longTimer += fElapsedTime;

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
            if (timer >= 0.4f) {
                timer = 0.0f;

                float randomAngle = RandomF(-pi, pi);
                // sx = either -600.0f or 600.0f
                float sx = RandomF(-1.0f, 1.0f) < 0.0f ? -600.0f : 600.0f;
                float sy = RandomF(-1.0f, 1.0f) * 400.0f;
                olc::vf2d pos = olc::vf2d{sx, sy} + player->position;

                auto playerSize = player->figure.Size();

                // 15% chance of spawning a tank
                float chance = RandomF(0.0f, 1.0f);

                if (chance < 0.15f) {
                    auto t = Add<Tank>(this);
                    t->position = pos;
                } else {
                    auto s = Add<Swarmer>(this);
                    s->position = pos;
                }
            }
        }

        tweenAnimator.Update(fElapsedTime);

        auto& gd = *m_pGlobalData.get();

        auto& draw = pge->GetDraw();

        draw.Clear(olc::Colour::BLACK);

        std::stable_sort(entities.begin(), entities.end(), [](const auto& a, const auto& b) {
            return a->position.y < b->position.y;
        });

        // Update camera
        auto diff = player->position - camera;
        camera += diff * fElapsedTime * 3.0f;

        auto cameraPos = camera - pge->ScreenSize() / 2.0f;
        DrawFloor(pge, cameraPos - cameraShaker);
        draw.WorldOffset(-cameraPos + cameraShaker);

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

        // Keep entities inside the floor
        for (auto& e : entities) {
            if (!e->resolveCollision) continue;
            e->position.x = std::clamp(e->position.x, kFloorMin.x+kTile*2, kFloorMax.x-kTile*2);
            e->position.y = std::clamp(e->position.y, kFloorMin.y+kTile*2, kFloorMax.y-kTile*2);
        }

        // Remove dead entities
        entities.erase(std::remove_if(entities.begin(), entities.end(),
            [](const auto& e) { return e->IsDestroyed(); }), entities.end());

        comboDisplay.Update(pge, fElapsedTime);
        weaponShuffler.Update(pge, fElapsedTime);
        damageDisplay.Update(pge, fElapsedTime);

        // FX
        if (player->health <= gCriticalPlayerHealth) {
            draw.SetTarget(fxRedVignetteTex);
            draw.Clear(olc::Colour::BLACK);
            draw.SetShader(fxRedVignetteShader);

            float intensity = float(gCriticalPlayerHealth - player->health) / float(gCriticalPlayerHealth);
            float pulsating = (0.5f + 0.5f * std::sin(longTimer * 6.0f)) * 0.15f;
            draw.SetShaderUniform("uIntensity", intensity + pulsating);
            draw.FilledRect({0,0}, pge->ScreenSize());
            draw.ResetShader();

            draw.SetTarget(pge->GetScreen());

            draw.SetBlendMode(olc::BlendMode::Additive);
            draw.Image(fxRedVignetteTex.all(), {0,0});
            draw.SetBlendMode(olc::BlendMode::Alpha);
        }


        // show GD stats (debug)
        // draw.StringProp(
        //     { 8, 8 },
        //     "Score: " + std::to_string(gd.score) + "\n"
        //     "Hits: " + std::to_string(gd.hits) + "\n"
        //     "Speed Mul.: " + std::to_string(gd.moveSpeedMultiplier) + "\n"
        //     "Fire Rate Mul.: " + std::to_string(gd.fireRateMultiplier) + "\n"
        //     "Score Mul.: " + std::to_string(gd.scoreMultiplier) + "\n"
        //     "Combo: " + std::to_string(gd.combo) + "\n"
        //     "Swarmers: " + std::to_string(swarmers) + "\n"
        //     "Swap Countdown: " + std::to_string(weaponShuffler.timer) + "\n"
        //     "Health: " + std::to_string(Get<Player>()->health),
        //     olc::Colour::WHITE
        // );

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

    // Draws floor.png as 32x32 world tiles on a ground plane tilted around X.
    // Each tile is a trapezoid (two textured triangles); light is per vertex, summed over `lights`.
    void DrawFloor(olc::PixelGameEngine* pge, const olc::vf2d& cameraPos)
    {
        auto& draw = pge->GetDraw();
        auto* tex = ImageRepository::Get().GetImage("assets/sprites/floor.png");
        if (!tex) return;

        const auto screen = pge->ScreenSize();
        const float cx = cameraPos.x + screen.x / 2.0f;
        const float depthK = screen.y / (1.0f / kFar - 1.0f); // keeps the visible world height unchanged

        // World (ground plane) to screen. Scale is 1/(1 + depth/K): linear in screen y, as for a real ground plane.
        auto project = [&](float wx, float wy) {
            const float scale = 1.0f / (1.0f + (cameraPos.y + screen.y - wy) / depthK);
            return olc::vf2d{
                screen.x / 2.0f + (wx - cx) * scale,
                (scale - kFar) / (1.0f - kFar) * screen.y
            };
        };
        auto light = [&](float wx, float wy) {
            float f = 0.0f;
            for (const auto& l : lights) {
                const float k = std::clamp(1.0f - (olc::vf2d{wx, wy} - l.pos).mag() / l.radius, 0.0f, 1.0f);
                f += k * k;
            }
            const float m = std::min(1.0f, kAmbient + (1.0f - kAmbient) * f);
            return olc::Pixel(uint8_t(255 * m), uint8_t(255 * m), uint8_t(255 * m));
        };

        const int tx0 = std::max(-kFloorTiles.x / 2, int(std::floor((cx - screen.x / 2.0f / kFar) / kTile)));
        const int tx1 = std::min(kFloorTiles.x / 2, int(std::ceil((cx + screen.x / 2.0f / kFar) / kTile)));
        const int ty0 = std::max(-kFloorTiles.y / 2, int(std::floor(cameraPos.y / kTile)));
        const int ty1 = std::min(kFloorTiles.y / 2, int(std::ceil((cameraPos.y + screen.y) / kTile)));

        for (int ty = ty0; ty < ty1; ty++) {
            for (int tx = tx0; tx < tx1; tx++) {
                const float x0 = float(tx * kTile), x1 = x0 + kTile;
                const float y0 = float(ty * kTile), y1 = y0 + kTile;
                const auto tl = project(x0, y0), tr = project(x1, y0);
                const auto bl = project(x0, y1), br = project(x1, y1);
                const auto ltl = light(x0, y0), ltr = light(x1, y0);
                const auto lbl = light(x0, y1), lbr = light(x1, y1);

                draw.TexturedTriangle(tl, tr, bl, ltl, ltr, lbl, {0, 0}, {1, 0}, {0, 1}, *tex);
                draw.TexturedTriangle(tr, br, bl, ltr, lbr, lbl, {1, 0}, {1, 1}, {0, 1}, *tex);
            }
        }
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

    void AwardScore(int amt = 5)
    {
        auto& gd = *m_pGlobalData.get();

        gd.score += amt * gd.scoreMultiplier;
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

    olc::Pixel GetLightContributionAt(const olc::vf2d& pos)
    {
        float f = 0.0f;
        for (const auto& l : lights) {
            const float k = std::clamp(1.0f - (pos - l.pos).mag() / l.radius, 0.0f, 1.0f);
            f += k * k;
        }
        const float m = std::min(1.0f, kAmbient + (1.0f - kAmbient) * f);
        return olc::Pixel(uint8_t(255 * m), uint8_t(255 * m), uint8_t(255 * m));
    }

    struct Light {
        olc::vf2d pos;
        float radius;
    };
    std::vector<Light> lights;

    std::vector<std::unique_ptr<Entity>> entities;
    std::vector<std::unique_ptr<Entity>> entityQueue;

    float timer{0.0f};

    ComboDisplay comboDisplay;
    WeaponShuffler weaponShuffler;
    DamageDisplay damageDisplay;

    olc::vf2d cameraShaker{0.0f, 0.0f}, camera{0.0f, 0.0f};

    TweenAnimator tweenAnimator;

    float longTimer{0.0f};

    // FX
    olc::gpu::Shader_GLSL33 fxRedVignetteShader;
    olc::Image fxRedVignetteTex;
};
