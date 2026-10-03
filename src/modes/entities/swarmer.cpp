#include "swarmer.h"

#include "../in_game_state.hpp"
#include "bullet.h"

#include "utilities/olcUTIL3_Geometry2D.h"
namespace g2d = olc::utils::geom2d;

Swarmer::Swarmer(InGameState *game) : game(game) {
    auto fig = FigureRepository::Get().GetFigure("assets/swarmer.stk");
    if (fig) {
        figure = *fig;
        colliderRadius = figure.Size().x / 3.4f;
    }
    mass = 5.0f;
}

void Swarmer::OnUpdate(olc::PixelGameEngine *pge, float fElapsedTime)
{
    auto& draw = pge->GetDraw();
    auto target = game->Get<Player>();

    const auto targetVec = (target->position - position);
    const auto dirToTarget = targetVec.norm();
    bool flipX = dirToTarget.x < 0.0f;

    auto size = figure.Size();
    auto renderPos = position - olc::vf2d{0.0f, size.y/2.6f};

    auto hitBox = g2d::rect<float>{
        { renderPos.x - colliderRadius, renderPos.y - colliderRadius*1.4f },
        { colliderRadius*2.0f, colliderRadius*2.8f }
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
            position += dirToTarget * fElapsedTime * game->Params().swarmerSpeed;
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
            if (!m_hasHit && state == State::ATTACK && figure.GetCurrentFrame("attack") >= 9) {
                m_hasHit = true;
                if (dist <= game->Params().attackDistance * 1.5f) target->TakeDamage(game->Params().swarmerDamage);
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

            // ouch sounds
            constexpr const char* sounds[] = {
                "assets/sounds/swarmer-ouch1.wav",
                "assets/sounds/swarmer-ouch2.wav"
            };
            auto soundFile = sounds[RandomI(0, 1)];

            float pan = std::clamp((position.x - target->position.x) / 300.0f, -1.0f, 1.0f);
            SRG(soundFile)->Play(false, 0.5f, pan, RandomF(0.8f, 1.2f));

            state = State::DEAD;
            figure.PlayAnimation("death");
            game->AwardScore(game->Params().swarmerScore);
        }
    });

    auto light = game->GetLightContributionAt(position);

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
