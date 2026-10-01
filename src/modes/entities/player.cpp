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

    auto mp = mouse.GetPosition();
    auto dir = (mp - position).norm();
    auto flipX = dir.x < 0.0f;

    olc::vf2d moveDir{0.0f, 0.0f};

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

    auto size = figure.Size();
    auto renderPos = position - olc::vf2d{0.0f, size.y/2.6f};

    auto fnSpawnBullet = [&](const Weapon& w, uint32_t bulletNo) {
        auto bullet = game->Add<PlayerBullet>(*w.bulletAsset);
        float bulletFacing = flipX ? -1.0f : 1.0f;

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
    if (mouse.GetButton(0).bHeld) {
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

    auto tmp = draw.GetWorldTransform();
    draw.WorldOffset(renderPos);
    figure.Draw(draw, fElapsedTime, flipX);
    draw.SetWorldTransform(tmp);
}

void Player::SwapWeapon()
{
    const auto weaponCount = sizeof(gWeapons) / sizeof(gWeapons[0]);
    float randomW = RandomF(0.0f, float(weaponCount));
    weapon = uint32_t(std::min(size_t(randomW), weaponCount - 1));
}
