#include "player.h"

#include "bullet.h"
#include "../in_game_state.hpp"

#include "../../utils.hpp"

#define IRG(x) ImageRepository::Get().GetSprite(x)

Player::Player(InGameState *game) : game(game)
{
    auto fig = FigureRepository::Get().GetFigure("assets/player.stk");
    if (fig) {
        figure = *fig;
        colliderRadius = figure.Size().x / 3.0f;
    }
    mass = 70.0f;
}

void Player::OnCreate(olc::PixelGameEngine* pge)
{
    figure.PlayAnimation("idle");

    for (auto& w : gWeapons) {
        if (w.bulletAsset || w.equipAsset) continue;
        w.equipAsset = IRG(w.equipAssetPath);
        w.bulletAsset = IRG(w.bulletAssetPath);
    }
}

void Player::OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime)
{
    auto& mouse = pge->GetMouse();
    auto& keyboard = pge->GetKeyboard();
    auto& draw = pge->GetDraw();

    auto mp = draw.ScreenToWorld(mouse.GetPosition());
    auto dir = (mp - position).norm();

    olc::vf2d moveDir{0.0f, 0.0f};

    if (!IsDead()) {
        m_flipX = dir.x < 0.0f;
        if (keyboard.GetKey(olc::Key::W).bHeld) {
            figure.PlayAnimation("run");
            moveDir.y = -1.0f;
        } else if (keyboard.GetKey(olc::Key::S).bHeld) {
            figure.PlayAnimation("run");
            moveDir.y = 1.0f;
        }

        if (keyboard.GetKey(olc::Key::A).bHeld) {
            figure.PlayAnimation("run");
            moveDir.x = -1.0f;
        } else if (keyboard.GetKey(olc::Key::D).bHeld) {
            figure.PlayAnimation("run");
            moveDir.x = 1.0f;
        }

        float moveSpeedMult = 1.0f + float(game->GameData().moveSpeedMultiplier) / 100.0f;

        if (moveDir.mag2() > 0.0f)
            position += moveDir.norm() * fElapsedTime * 140.0f * moveSpeedMult;
        else figure.PlayAnimation("idle");

        m_healthRechargeTimer += fElapsedTime;
        if (m_healthRechargeTimer >= 1.0f) {
            m_healthRechargeTimer = 0.0f;
            health = std::min(health + 1, gPlayerMaxHealth);
        }
    }

    auto size = figure.Size();
    auto renderPos = position - olc::vf2d{0.0f, size.y/2.6f};

    auto fnSpawnBullet = [&](const Weapon& w, uint32_t bulletNo) {
        auto bullet = game->Add<PlayerBullet>(*w.bulletAsset);
        float bulletFacing = m_flipX ? -1.0f : 1.0f;

        auto localWPos = figure.GetStickWorldTipOffset(figure.GetStickID("weapon"));
        localWPos.x *= bulletFacing;

        auto pos = renderPos + localWPos;

        const float halfAngle = Deg2Rad(w.spread) / 2.0f;
        float angleFactor = w.numProjectiles > 1
            ? ((float(bulletNo) / float(w.numProjectiles)) * 2.0f - 1.0f) * halfAngle
            : RandomF(-halfAngle, halfAngle);

        olc::vf2d direction = RotateVector(olc::vf2d{ bulletFacing, 0.0f }, angleFactor);

        bullet->position = pos;
        bullet->direction = direction;
        bullet->speed = RandomF(w.projectileSpeed * 0.8f, w.projectileSpeed * 1.2f);
        bullet->damage = w.damage;
        bullet->gravity = w.gravity;
        bullet->floorY = int(position.y);
        bullet->colliderRadius = w.collisionRadius;
    };

    const auto& w = gWeapons[weapon];
    if (mouse.GetButton(0).bHeld && !IsDead()) {
        shootTimer += fElapsedTime;

        const auto& gd = game->GameData();
        float fireRateMult = float(100 - gd.fireRateMultiplier) / 100.0f;

        if (shootTimer >= w.fireRate * fireRateMult) {
            shootTimer = 0.0f;

            for (uint32_t i = 0; i < w.numProjectiles; i++) {
                fnSpawnBullet(w, i);
            }

            game->ShakeCamera(1.5f);
        }
    }

    // update weapon sprite
    auto stk = figure.GetStick(figure.GetStickID("weapon"));
    if (stk && stk->sprite.image != w.equipAsset->image)
        stk->sprite = *w.equipAsset;

    m_damageColorTimer -= fElapsedTime;
    if (m_damageColorTimer <= 0.0f) {
        m_damageColorTimer = 0.0f;
    }

    float t = m_damageColorTimer / 0.25f;
    olc::Pixel color = olc::PixelLerp(olc::Colour::WHITE, olc::PixelF(1.0f, 0.5f, 0.5f), t);

    auto light = game->GetLightContributionAt(position);

    auto tmp = draw.GetWorldTransform();
    draw.WorldOffset(renderPos);
    figure.Draw(draw, fElapsedTime, m_flipX, color, light);
    draw.SetWorldTransform(tmp);
}

void Player::SwapWeapon()
{
    const auto weaponCount = sizeof(gWeapons) / sizeof(gWeapons[0]);
    float randomW = RandomF(0.0f, float(weaponCount));
    weapon = uint32_t(std::min(size_t(randomW), weaponCount - 1));
}

void Player::TakeDamage(int value)
{
    if (IsDead()) return;
    health -= value;

    m_damageColorTimer = 0.25f;
    m_healthRechargeTimer = 0.0f;

    if (game->GameData().combo > 0) {
        game->damageDisplay.Bump();
        if (++damageCounter >= 3) {
            game->comboDisplay.Reset();
            game->damageDisplay.Reset();
        }
    }

    if (health <= 0) {
        health = 0;
        figure.PlayAnimation("death");
        game->ShakeCamera(4.0f);
        if (onDeath) onDeath();
    }
}
