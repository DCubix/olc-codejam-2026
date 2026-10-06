#pragma once

#include <iostream>

#include "../tween.h"

#include "../logic.h"
#include "../difficulty.hpp"
#include "../repository.h"

#include "../gui.h"

#include "entities/player.h"
#include "entities/swarmer.h"
#include "entities/tank.h"
#include "../utils.hpp"
#include "hud/combodisplay.h"
#include "hud/weaponshuffler.h"
#include "hud/damagedisplay.h"
#include "hud/statsdisplay.h"

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
    InGameState(Difficulty difficulty = Difficulty::NORMAL) : difficulty(difficulty) {}
    ~InGameState() = default;

    InGameState(std::shared_ptr<GlobalGameData> pGlobalData, Difficulty difficulty = Difficulty::NORMAL)
        : GameMode(pGlobalData), difficulty(difficulty)
    {

    }

    Difficulty difficulty;
    const DifficultyParams& Params() const { return GetDifficulty(difficulty); }

    GlobalGameData& GameData() { return *m_pGlobalData.get(); }

public:

    bool OnCreate(olc::PixelGameEngine* pge) override
    {
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

        ui = gui::State(pge);

        return false;
    }

    bool OnEnterMode(olc::PixelGameEngine* pge) override
    {
        m_pGlobalData = std::make_shared<GlobalGameData>();

        auto player = Add<Player>(this);
#ifndef NDEBUG
        player->health = Params().playerCriticalHealth;
#endif

        comboDisplay.data = m_pGlobalData.get();
        comboDisplay.params = &Params();
        comboDisplay.onExpire = [this]() {
            auto& gd = *m_pGlobalData;
            gd.hits = 0;
            gd.fireRateMultiplier = 0;
            gd.moveSpeedMultiplier = 0;
            gd.scoreMultiplier = 1;
            weaponShuffler.RequestStop();

            // timed out (not "combo lost" by damage): clear the damage counter and its HUD
            if (auto p = Get<Player>(); p && p->damageCounter < Params().damageHitsToLoseCombo) {
                p->damageCounter = 0;
                damageDisplay.Hide();
            }
        };

        weaponShuffler.params = &Params();
        weaponShuffler.onSwap = [this]() {
            // no swap once the combo is over
            if (!comboDisplay.Active()) return false;
            if (auto p = Get<Player>()) {
                p->SwapWeapon();
                weaponShuffler.selectedWeapon = p->weapon;
            }
            return true;
        };

        player->onDeath = [this]() {
            weaponShuffler.RequestStop();
            comboDisplay.Reset();
            subState = SubState::DYING;
        };

        paused = false;
        subState = SubState::PLAYING;
        deathTimer = 0.0f;

        damageDisplay.player = player;
        damageDisplay.params = &Params();

        statsDisplay.player = player;
        statsDisplay.params = &Params();
        statsDisplay.data = m_pGlobalData.get();

        // Lights
        const float lightMargin = 2.0f * kTile;
        for (int i = 0; i < 18; i++) {
            Light l;
            l.pos = olc::vf2d{
                RandomF(kFloorMin.x + lightMargin, kFloorMax.x - lightMargin),
                RandomF(kFloorMin.y + lightMargin, kFloorMax.y - lightMargin)
            };
            l.radius = RandomF(80.0f, 300.0f);
            l.color = olc::PixelF(
                RandomF(0.5f, 1.0f),
                RandomF(0.5f, 1.0f),
                RandomF(0.0f, 0.1f),
                1.0f
            );
            lights.push_back(l);
        }

        // Light that follows player
        Light playerLight;
        playerLight.pos = player->position;
        playerLight.radius = 180.0f;
        playerLight.color = olc::PixelF(1.0f, 0.7f, 0.0f, 1.0f);
        lights.push_back(playerLight);

        return false;
    }

    bool OnExitMode(olc::PixelGameEngine* pge) override
    {
        // free all
        SetPaused(false);
        entities.clear();
        entityQueue.clear();
        lights.clear();
        m_pGlobalData.reset();
        return false;
    }

    PlayState OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) override
    {
        ui.Update();

        auto& keys = pge->GetKeyboard();
        if (subState == SubState::PLAYING &&
            (keys.GetKey(olc::Key::ESCAPE).bPressed || keys.GetKey(olc::Key::P).bPressed)) {
            SetPaused(!paused);
        }

        // Add queued entities
        for (auto& e : entityQueue) {
            entities.push_back(std::move(e));
        }
        entityQueue.clear();

        if (subState == SubState::PLAYING) {
            if (!paused) UpdateLogic(pge, fElapsedTime);
        } else if (subState == SubState::DYING) {
            // slow motion; the timer counts real time
            UpdateLogic(pge, fElapsedTime * 0.5f);
            deathTimer += fElapsedTime;
            if (deathTimer >= kDeathSlowmoDuration) {
                SRG("assets/sounds/bam.wav")->Play(false, 0.8f, 0.0f, 0.6f);
                subState = SubState::GAME_OVER;
            }
        }
        // GAME_OVER: no logic update, the last frame stays on screen

        OnDraw(pge);

        // UI
        PlayState next = PlayState::IN_GAME;
        if (paused) {
            auto& draw = pge->GetDraw();
            auto size = draw.GetTargetSize();
            draw.FilledRect({0, 0}, size, olc::PixelF(0.0f, 0.0f, 0.0f, 0.6f));

            constexpr int kPauseWidth = 160;
            constexpr int kPauseHeight = 100;
            gui::ContainerRect(ui, {{size.x / 2 - kPauseWidth / 2, size.y / 2 - kPauseHeight / 2}, {kPauseWidth, kPauseHeight}});
            gui::Label(ui, gui::ContainerTop(ui, 24), "PAUSED", {2.0f, 2.0f});
            if (gui::Button(ui, gui::ContainerTop(ui, 24), "resume", "Resume")) {
                SetPaused(false);
            }
            gui::ContainerTop(ui, 4); // gap
            if (gui::Button(ui, gui::ContainerTop(ui, 24), "back_menu", "Back to Menu")) {
                next = PlayState::MENU;
            }
            gui::ContainerPop(ui);
        }

        if (subState == SubState::GAME_OVER) {
            auto& draw = pge->GetDraw();
            auto size = draw.GetTargetSize();
            draw.FilledRect({0, 0}, size, olc::PixelF(0.0f, 0.0f, 0.0f, 0.6f));

            constexpr int kWidth = 160;
            constexpr int kHeight = 100;
            bool restart = false;
            gui::ContainerRect(ui, {{size.x / 2 - kWidth / 2, size.y / 2 - kHeight / 2}, {kWidth, kHeight}});
            gui::Label(ui, gui::ContainerTop(ui, 24), "GAME OVER", {2.0f, 2.0f});
            if (gui::Button(ui, gui::ContainerTop(ui, 24), "play_again", "Play Again")) {
                restart = true;
            }
            gui::ContainerTop(ui, 4); // gap
            if (gui::Button(ui, gui::ContainerTop(ui, 24), "game_over_menu", "Back to Menu")) {
                next = PlayState::MENU;
            }
            gui::ContainerPop(ui);

            // the new player is moved into `entities` at the start of the next update, before it is used
            if (restart) {
                OnExitMode(pge);
                OnEnterMode(pge);
            }
        }

        return next;
    }

    // World to screen offset: screen position + CameraOffset = world position.
    olc::vf2d CameraOffset(olc::PixelGameEngine* pge) const
    {
        return camera - pge->ScreenSize() / 2.0f - cameraShaker;
    }

    olc::vf2d ScreenToWorld(olc::PixelGameEngine* pge, const olc::vf2d& screenPos) const
    {
        return screenPos + CameraOffset(pge);
    }

    void UpdateLogic(olc::PixelGameEngine* pge, float fElapsedTime)
    {
        auto player = Get<Player>();

        longTimer += fElapsedTime;

#ifndef NDEBUG
        // DEBUG
        if (pge->GetKeyboard().GetKey(olc::Key::K1).bPressed) { // simulate countdown at 5 seconds
            weaponShuffler.timer = 5.0f;
            weaponShuffler.Resume();
        }
        //
#endif

        int enemies = GetEntityCount<Swarmer, Tank>();
        if (enemies < Params().maxSwarmers) {
            timer += fElapsedTime;
            int spawnCut = std::clamp(std::max(0, m_pGlobalData->combo - 1) * Params().spawnRateStepPerCombo,
                                      0, Params().maxFireRateMultiplier);
            if (timer >= Params().spawnInterval * float(100 - spawnCut) / 100.0f) {
                timer = 0.0f;

                constexpr olc::vf2d spawnLocations[] = {
                    // 4 floor corners
                    {kFloorMin.x + kTile*2, kFloorMin.y + kTile*2},
                    {kFloorMax.x - kTile*2, kFloorMin.y + kTile*2},
                    {kFloorMin.x + kTile*2, kFloorMax.y - kTile*2},
                    {kFloorMax.x - kTile*2, kFloorMax.y - kTile*2}
                };

                int corner = RandomI(0, 3);
                olc::vf2d pos = spawnLocations[corner];

                float chance = RandomF(0.0f, 1.0f);

                if (chance < Params().tankChance) {
                    auto t = Add<Tank>(this);
                    t->position = pos;
                } else {
                    auto s = Add<Swarmer>(this);
                    s->position = pos;
                }
            }
        }

        tweenAnimator.Update(fElapsedTime);

        std::stable_sort(entities.begin(), entities.end(), [](const auto& a, const auto& b) {
            return a->position.y < b->position.y;
        });

        // Update camera
        auto diff = player->position - camera;
        camera += diff * std::min(fElapsedTime * 3.0f, 1.0f);

        for (auto& e : entities) {
            e->Update(pge, fElapsedTime);
        }

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
        statsDisplay.Update(pge, fElapsedTime);

        // FX
        vignetteIntensity = 0.0f;
        if (player->health <= Params().playerCriticalHealth) {
            float intensity = float(Params().playerCriticalHealth - player->health) / float(Params().playerCriticalHealth);
            float pulsating = (0.5f + 0.5f * std::sin(longTimer * 6.0f)) * 0.15f;
            vignetteIntensity = intensity + pulsating;

            // play heart beat sound when pulsating
            if (intensity > 0.0f && pulsating < 0.01f) {
                SRG("assets/sounds/heartbeat.wav")->Play(false, 0.5f, 0.0f, RandomF(0.9f, 1.1f));
            }
        }

        // move light to player
        lights.back().pos = player->position;
    }

    void OnDraw(olc::PixelGameEngine* pge)
    {
        auto& draw = pge->GetDraw();

        draw.Clear(olc::Colour::BLACK);

        auto cameraOffset = CameraOffset(pge);
        DrawFloor(pge, camera - pge->ScreenSize() / 2.0f - cameraShaker);
        draw.WorldOffset(-cameraOffset);

        for (auto& e : entities) {
            e->Draw(pge);
        }
        draw.WorldReset();

        comboDisplay.Draw(pge);
        weaponShuffler.Draw(pge);
        damageDisplay.Draw(pge);
        statsDisplay.Draw(pge);

        // FX
        if (Get<Player>()->health <= Params().playerCriticalHealth) {
            draw.SetTarget(fxRedVignetteTex);
            draw.Clear(olc::Colour::BLACK);
            draw.SetShader(fxRedVignetteShader);
            draw.SetShaderUniform("uIntensity", vignetteIntensity);
            draw.FilledRect({0,0}, pge->ScreenSize());
            draw.ResetShader();

            draw.SetTarget(pge->GetScreen());

            draw.SetBlendMode(olc::BlendMode::Additive);
            draw.Image(fxRedVignetteTex.all(), {0,0});
            draw.SetBlendMode(olc::BlendMode::Alpha);
        }
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
        auto light = [&](float wx, float wy) { return GetLightContributionAt({wx, wy}); };

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

    template <typename T>
    uint32_t GetEntityCount()
    {
        const int id = TypeIdOf<T>();
        uint32_t count = 0;
        for (auto& e : entities)
            if (e->TypeId() == id) count++;
        return count;
    }

    template <typename T, typename U, typename... Rest>
    uint32_t GetEntityCount()
    {
        return GetEntityCount<T>() + GetEntityCount<U, Rest...>();
    }

    // Pauses (or resumes) playing sounds too, so overlapping one-shots freeze with the scene.
    void SetPaused(bool value)
    {
        if (paused == value) return;
        paused = value;
        SoundRepository::Get().SetPaused(paused);
    }

    void AwardScore(int amt = 5)
    {
        auto& gd = *m_pGlobalData.get();

        gd.score += amt * gd.scoreMultiplier;
        gd.hits++;

        ShakeCamera(3.5f);

        if (gd.hits % Params().comboEveryNHits != 0) return;
        if (Get<Player>()->IsDead()) return;

        gd.combo++;

        if (gd.combo > 1) {
            comboDisplay.Bump();
            weaponShuffler.Resume();
            if (gd.combo == 2) weaponShuffler.ResetTimer();

            constexpr float comboPitches[] = { 0.6f, 0.8f, 1.0f, 1.2f, 1.4f };
            float pitch = comboPitches[std::min(gd.combo - 2, 4)];

            SRG("assets/sounds/combo.wav")->Play(false, 0.5f, 0.0f, pitch);

            gd.fireRateMultiplier += Params().fireRateStepPerCombo;
            gd.moveSpeedMultiplier += Params().moveSpeedStepPerCombo;
            gd.scoreMultiplier += Params().scoreStepPerCombo;

            gd.fireRateMultiplier = std::clamp(gd.fireRateMultiplier, 0, Params().maxFireRateMultiplier);
            gd.moveSpeedMultiplier = std::clamp(gd.moveSpeedMultiplier, 0, Params().maxMoveSpeedMultiplier);
            gd.scoreMultiplier = std::clamp(gd.scoreMultiplier, 1, Params().maxScoreMultiplier);
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

    // Ambient plus each light's color weighted by a squared falloff, per channel.
    olc::Pixel GetLightContributionAt(const olc::vf2d& pos)
    {
        float r = 0.0f, g = 0.0f, b = 0.0f;
        for (const auto& l : lights) {
            const float k = std::clamp(1.0f - (pos - l.pos).mag() / l.radius, 0.0f, 1.0f);
            const float w = k * k / 255.0f;
            r += w * l.color.r;
            g += w * l.color.g;
            b += w * l.color.b;
        }
        auto channel = [](float f) {
            return uint8_t(255 * std::min(1.0f, kAmbient + (1.0f - kAmbient) * f));
        };
        return olc::Pixel(channel(r), channel(g), channel(b));
    }

    struct Light {
        olc::vf2d pos;
        float radius;
        olc::Pixel color;
    };
    std::vector<Light> lights;

    std::vector<std::unique_ptr<Entity>> entities;
    std::vector<std::unique_ptr<Entity>> entityQueue;

    float timer{0.0f};
    bool paused{false};

    enum class SubState { PLAYING, DYING, GAME_OVER };
    SubState subState{SubState::PLAYING};
    float deathTimer{0.0f};
    static constexpr float kDeathSlowmoDuration = 2.0f; // real seconds of slow motion before the game over screen

    ComboDisplay comboDisplay;
    WeaponShuffler weaponShuffler;
    DamageDisplay damageDisplay;
    StatsDisplay statsDisplay;

    olc::vf2d cameraShaker{0.0f, 0.0f}, camera{0.0f, 0.0f};

    TweenAnimator tweenAnimator;

    float longTimer{0.0f};
    float vignetteIntensity{0.0f};

    // FX
    olc::gpu::Shader_GLSL33 fxRedVignetteShader;
    olc::Image fxRedVignetteTex;

    // UI
    gui::State ui;
};
