#include "tank.h"

#include "../in_game_state.hpp"
#include "bullet.h"

#include "utilities/olcUTIL3_Geometry2D.h"
namespace g2d = olc::utils::geom2d;

Tank::Tank(InGameState *game) : game(game)
{
    auto fig = FigureRepository::Get().GetFigure("assets/tank.stk");
    if (fig) {
        figure = *fig;
        colliderRadius = figure.Size().x / 3.4f;
    }
    mass = 12.0f;
    health = game->Params().tankHealth;
}

void Tank::OnUpdate(olc::PixelGameEngine *pge, float fElapsedTime)
{
    auto& draw = pge->GetDraw();
    auto target = game->Get<Player>();

    const auto targetVec = (target->position - position);
    const auto dirToTarget = targetVec.norm();
    bool flipX = dirToTarget.x < 0.0f;

    auto size = figure.Size();
    auto renderPos = position - olc::vf2d{0.0f, size.y/2.6f};

    auto hitBox = g2d::rect<float>{
        { renderPos.x - colliderRadius, renderPos.y - colliderRadius*1.25f },
        { colliderRadius*2.0f, colliderRadius*2.5f }
    };

    switch (state) {
    case State::TO_IDLE:
        figure.PlayAnimation("idle");
        state = State::IDLE;
        break;
    case State::IDLE: {
        if (targetVec.mag() < game->Params().chaseDistance) {
            figure.PlayAnimation("walk");
            state = State::CHASING;
        }
    } break;
    case State::CHASING: {
        if (targetVec.mag() > game->Params().chaseDistance) {
            state = State::TO_IDLE;
        } else if (targetVec.mag() <= game->Params().attackDistance) {
            figure.PlayAnimation("attack");
            state = State::ATTACK;
        }
        position += dirToTarget * fElapsedTime * game->Params().tankSpeed;
    } break;
    case State::ATTACK: {
        if (figure.IsAnimationFinished("attack") && targetVec.mag() > game->Params().attackDistance) {
            figure.PlayAnimation("walk");
            state = State::CHASING;
        }

        const bool onHitFrame = figure.GetCurrentFrame("attack") == 5;
        if (onHitFrame && !m_hasHit) target->TakeDamage(game->Params().tankDamage);
        m_hasHit = onHitFrame;
    } break;
    case State::DEAD: {
        if (figure.IsAnimationFinished("death")) {
            Destroy();
        }
    } break;
    default: break;
    }

    // handle player bullet collisions
    game->ForEachEntityOfType<PlayerBullet>([=](PlayerBullet& b) {
        if (IsDestroyed()) return;

        auto bulletHitCircle = g2d::circle<float>{b.position, b.colliderRadius};

        if (g2d::overlaps(hitBox, bulletHitCircle) && state != State::DEAD) {
            b.Destroy();
            if (health-- <= 0) {
                health = 0;
                state = State::DEAD;
                figure.PlayAnimation("death");
                game->AwardScore(game->Params().tankScore);
            }
            m_damageColorTimer = 0.25f;
        }
    });

    auto light = game->GetLightContributionAt(position);

    m_damageColorTimer -= fElapsedTime;
    if (m_damageColorTimer <= 0.0f) {
        m_damageColorTimer = 0.0f;
    }

    float t = m_damageColorTimer / 0.25f;
    light = olc::PixelLerp(light, olc::PixelF(1.0f, 0.5f, 0.5f), t);

    auto tmp = draw.GetWorldTransform();
    draw.WorldOffset(position - olc::vf2d{0.0f, size.y/2.4f});
    figure.Draw(draw, fElapsedTime, flipX, {}, light);
    draw.SetWorldTransform(tmp);

    // draw.Rect(
    //     hitBox.pos,
    //     hitBox.size,
    //     olc::Pixel(255, 0, 0)
    // );
}
