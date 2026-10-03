#include "weaponshuffler.h"

#include <cmath>

#include "../entities/player.h"

void WeaponShuffler::OnShow() {}

void WeaponShuffler::OnHide()
{
    m_state = State::IDLE;
    m_stopping = false;
}

void WeaponShuffler::RequestStop()
{
    if (m_state == State::HOLD || m_state == State::HIDE) m_stopping = true;
    else Hide();
}

void WeaponShuffler::SetState(State s)
{
    m_state = s;
    m_stateTime = 0.0f;

    if (s == State::SHOW) {
        m_tweens.Animate(&m_frameSize)
            .From(m_minFrameSize).To(m_maxFrameSize).For(m_slideTime)
                .Ease(Ease::EaseOutBack)
                .Start();
    } else if (s == State::HIDE) {
        m_tweens.Animate(&m_frameSize)
            .From(m_maxFrameSize).To(m_minFrameSize).For(m_slideTime)
                .Ease(Ease::EaseInBack)
                .Start();
    }
}

void WeaponShuffler::OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime)
{
    m_tweens.Update(fElapsedTime);
    m_stateTime += fElapsedTime;
    timer -= fElapsedTime;

    switch (m_state) {
    case State::IDLE:
        if (timer <= params->shuffleLeadTime) SetState(State::SHOW);
        break;
    case State::SHOW:
        if (m_stateTime >= m_slideTime) SetState(State::SHUFFLE);
        break;
    case State::SHUFFLE:
        if (timer <= 0.0f) {
            if (onSwap && onSwap()) ResetTimer();
            else m_stopping = true;
            SetState(State::HOLD);
        }
        break;
    case State::HOLD:
        if (m_stateTime >= params->shuffleHoldTime) SetState(State::HIDE);
        break;
    case State::HIDE:
        if (m_stateTime < m_slideTime) break;
        if (m_stopping) Hide();
        else SetState(State::IDLE);
        break;
    }

    if (m_state == State::IDLE) return;

    auto& draw = pge->GetDraw();
    auto size = draw.GetTargetSize();

    const olc::vf2d frameSize = {m_maxFrameSize, m_frameSize};
    auto framePos = olc::vf2d{ size.x / 2.0f - frameSize.x / 2.0f, 10.0f };
    draw.FilledRect(framePos, frameSize, olc::PixelF(0.0f, 0.0f, 0.0f, 0.5f));
    draw.Rect(framePos, frameSize, olc::Colour::WHITE);

    if (m_state == State::SHUFFLE || m_state == State::HOLD) {
        float flashTime = std::fmodf(timer, 0.15f) / 0.15f;
        float flashValue = std::sinf(flashTime * pi * 2.0f);
        auto flashColor = flashValue > 0.0f ? olc::Colour::WHITE : olc::Colour::YELLOW;

        olc::vf2d sz = draw.GetTextSize("CHANGING", true);
        draw.StringProp(
            olc::vf2d{ framePos.x + frameSize.x / 2.0f - sz.x / 2.0f, framePos.y + 4.0f },
            "CHANGING",
            flashColor
        );

        constexpr uint32_t weaponCount = sizeof(gWeapons) / sizeof(gWeapons[0]);
        uint32_t weapon = selectedWeapon;
        if (m_state == State::SHUFFLE) {
            float t = std::fmodf(timer, 0.25f) / 0.25f;
            weapon = std::min(uint32_t(t * float(weaponCount)), weaponCount - 1);
        }
        const auto& w = gWeapons[weapon];
        const auto& weaponAsset = w.equipAsset;

        auto timeoutText = m_state == State::HOLD
            ? w.name
            : "IN " + std::to_string(int(std::ceilf(timer))) + "s";
        sz = draw.GetTextSize(timeoutText, true);
        draw.StringProp(
            olc::vf2d{ framePos.x + frameSize.x / 2.0f - sz.x / 2.0f, framePos.y + frameSize.y - sz.y - 4.0f },
            timeoutText,
            flashColor
        );

        auto weaponSize = weaponAsset->image->Size();
        auto weaponPos = framePos + olc::vf2d{ frameSize.x / 2.0f - weaponSize.x / 2.0f, frameSize.y / 2.0f - weaponSize.y / 2.0f };
        draw.Image(weaponAsset->image->all(), weaponPos);

        if (m_state == State::HOLD && flashValue > 0.0f) {
            draw.SetBlendMode(olc::BlendMode::Additive);
            draw.Image(weaponAsset->image->all(), weaponPos);
            draw.SetBlendMode(olc::BlendMode::Alpha);
        }
    }
}
