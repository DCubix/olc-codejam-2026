#include "bullet.h"

#include "../../utils.hpp"

#include <numbers>

PlayerBullet::PlayerBullet(const Sprite& spr)
{
    auto fig = FigureRepository::Get().GetFigure("assets/bullet.stk");
    if (fig) {
        figure = *fig;
        figure.GetStick(0)->sprite = spr;
    }
    resolveCollision = false;
    colliderRadius = figure.Size().x / 2.0f;
}

void PlayerBullet::OnCreate(olc::PixelGameEngine *pge)
{
    Destroy(1.5f);
}

void PlayerBullet::OnUpdate(olc::PixelGameEngine *pge, float fElapsedTime)
{
    auto& draw = pge->GetDraw();

    position += direction * fElapsedTime * speed;
    if (gravity) {
        direction = (direction + olc::vf2d{0.0f, 1.0f} * fElapsedTime * 2.0f).norm();
        if (position.y >= floorY) {
            Destroy();
        }
    }

    float angle = std::atan2(direction.y, direction.x) - M_PI_2;

    auto tmp = draw.GetWorldTransform();
    draw.WorldOffset(position);
    draw.WorldRotate(angle);
    figure.Draw(draw, fElapsedTime);
    draw.SetWorldTransform(tmp);
}
