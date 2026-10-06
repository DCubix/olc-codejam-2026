#include "damagedisplay.h"

void DamageDisplay::OnShow()
{
    m_tweens.Animate(&m_textOff.y)
        .From(15.0f).To(0.0f).For(0.5f)
            .Ease(Ease::EaseOutQuad)
            .Start();
}

void DamageDisplay::OnUpdate(olc::PixelGameEngine *pge, float fElapsedTime)
{
    m_blinkTimer = std::max(m_blinkTimer - fElapsedTime, 0.0f);
    m_tweens.Update(fElapsedTime);
}

void DamageDisplay::OnDraw(olc::PixelGameEngine *pge)
{
    auto& draw = pge->GetDraw();

    float flashTime = std::fmodf(m_blinkTimer, 0.15f) / 0.15f;
    float flashValue = std::sinf(flashTime * pi * 2.0f);
    auto color = (m_blinkTimer > 0.0f && flashValue > 0.0f) ? olc::Colour::WHITE : olc::Colour::RED;

    auto fnDrawBlock = [&](olc::vf2d pos, bool lit, olc::Pixel color, float width) {
        draw.Rect(
            { pos.x + 1, pos.y + 1 },
            { width, 5.0f },
            olc::Colour::BLACK
        );
        if (lit) {
            draw.FilledRect(
                pos,
                { width, 5.0f },
                color
            );
        }
        draw.Rect(
            pos,
            { width, 5.0f },
            olc::Colour::WHITE
        );
    };

    std::string damageText = player->damageCounter < params->damageHitsToLoseCombo
        ? "Damage!" : "Combo Lost :(";
    auto sz = draw.GetTextSize(damageText, true);
    auto pos = olc::vf2d{ gHUDPadding, sz.y + gHUDPadding };
    draw.StringProp(pos + olc::vf2d{ 1, 1 } + m_textOff, damageText, olc::Colour::BLACK);
    draw.StringProp(pos + m_textOff, damageText, olc::Colour::WHITE);

    constexpr float kSpacing = 3.0f;
    const int blocks = params->damageHitsToLoseCombo;
    const float blockWidth = (gDamageDisplayWidth / float(blocks)) - kSpacing;
    for (int i = 0; i < blocks; i++) {
        fnDrawBlock(pos + olc::vf2d{ (blockWidth + kSpacing) * float(i), sz.y + 4.0f + m_textOff.y }, player->damageCounter >= i + 1, color, blockWidth);
    }
}

void DamageDisplay::Bump()
{
    if (!IsVisible()) {
        Show();
    } else {
        m_tweens.Animate(&m_textOff.x)
            .From(4.0f).To(0.0f).For(0.3f)
                .Ease(Ease::EaseOutBack)
                .Start();
    }
    m_blinkTimer = 0.5f;
}

void DamageDisplay::Reset()
{
    if (m_exiting) return;
    m_exiting = true;
    if (onExpire) onExpire();

    m_tweens.Animate(&m_textOff.y)
        .Wait(2.0f)
        .From(0.0f).To(15.0f).For(0.4f)
        .Ease(Ease::EaseInCubic)
        .OnComplete([this]() {
            player->damageCounter = 0;
            m_exiting = false;
            Hide();
        })
        .Start();
}
