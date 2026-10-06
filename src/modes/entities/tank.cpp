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
    auto target = game->Get<Player>();

    const auto targetVec = (target->position - position);
    const auto dirToTarget = targetVec.norm();
    m_flipX = dirToTarget.x < 0.0f;

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
        state = State::CHASING;
    } break;
    case State::CHASING: {
        if (targetVec.mag() <= game->Params().attackDistance) {
            figure.PlayAnimation("attack");
            state = State::ATTACK;
            m_hasHit = false;
        } else {
            figure.PlayAnimation("walk");
        }
        position += dirToTarget * fElapsedTime * game->Params().tankSpeed;
    } break;
    case State::ATTACK: {
        const float dist = targetVec.mag();
        if (figure.IsAnimationFinished("attack")) {
            // the attack is a one-shot: restart it while the target is still close, otherwise chase
            figure.PlayAnimation("walk");
            if (dist > game->Params().attackDistance) state = State::CHASING;
            else { figure.PlayAnimation("attack"); m_hasHit = false; }
        }

        // hit once per swing; the range is looser than the trigger distance so a small step away during the wind-up still gets hit
        if (!m_hasHit && state == State::ATTACK && figure.GetCurrentFrame("attack") >= 5) {
            m_hasHit = true;
            if (dist <= game->Params().attackDistance * 1.5f) target->TakeDamage(game->Params().tankDamage);
        }
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
        if (IsDestroyed() || b.spent) return;

        auto bulletHitCircle = g2d::circle<float>{b.position, b.colliderRadius};

        if (g2d::overlaps(hitBox, bulletHitCircle) && state != State::DEAD) {
            b.HitSomething();

            float pan = std::clamp((position.x - target->position.x) / 300.0f, -1.0f, 1.0f);
            if (health-- <= 0) {
                health = 0;

                // death sounds
                constexpr const char* sounds[] = {
                    "assets/sounds/tank-death1.wav",
                    "assets/sounds/tank-death2.wav"
                };
                auto soundFile = sounds[RandomI(0, 1)];
                SRG(soundFile)->Play(false, 0.5f, pan, RandomF(0.8f, 1.2f));

                state = State::DEAD;
                figure.PlayAnimation("death");
                game->AwardScore(game->Params().tankScore);
            } else {
                SRG("assets/sounds/tank-hit.wav")->Play(false, 0.5f, pan, RandomF(0.8f, 1.2f));
            }
            m_damageColorTimer = 0.25f;
        }
    });

    m_damageColorTimer -= fElapsedTime;
    if (m_damageColorTimer <= 0.0f) {
        m_damageColorTimer = 0.0f;
    }

    figure.Update(fElapsedTime);
}

void Tank::OnDraw(olc::PixelGameEngine *pge)
{
    auto& draw = pge->GetDraw();
    auto size = figure.Size();
    auto light = game->GetLightContributionAt(position);

    float t = m_damageColorTimer / 0.25f;
    light = olc::PixelLerp(light, olc::PixelF(1.0f, 0.5f, 0.5f), t);

    auto tmp = draw.GetWorldTransform();
    draw.WorldOffset(position - olc::vf2d{0.0f, size.y/2.4f});
    figure.Draw(draw, m_flipX, {}, light);
    draw.SetWorldTransform(tmp);

    // draw.Rect(
    //     hitBox.pos,
    //     hitBox.size,
    //     olc::Pixel(255, 0, 0)
    // );
}
