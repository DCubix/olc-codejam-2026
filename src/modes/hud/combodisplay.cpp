#include "combodisplay.h"

#include <algorithm>
#include <string>

void ComboDisplay::OnShow()
{
    m_tweens.Animate(&m_textOff.y)
        .From(15.0f).To(0.0f).For(0.5f)
        .Ease(Ease::EaseOutQuad)
        .Start();
}

void ComboDisplay::Bump()
{
    m_timer = gComboResetTimer;

    if (!IsVisible()) {
        Show();
    } else {
        m_tweens.Animate(&m_textOff.x)
            .From(4.0f).To(0.0f).For(0.3f)
            .Ease(Ease::EaseOutBack)
            .Start();
    }
}

void ComboDisplay::Reset()
{
    m_timer = 0.0f;
    m_exiting = true;
    if (onExpire) onExpire();

    m_tweens.Animate(&m_textOff.y)
        .From(0.0f).To(15.0f).For(0.4f)
        .Ease(Ease::EaseInCubic)
        .OnComplete([this]() {
            data->combo = 0;
            m_exiting = false;
            Hide();
        })
        .Start();
}

void ComboDisplay::OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime)
{
    m_tweens.Update(fElapsedTime);

    m_timer = std::max(m_timer - fElapsedTime, 0.0f);
    if (!m_exiting && m_timer <= 0.0f && data->combo > 0) {
        Reset();
    }

    auto& draw = pge->GetDraw();
    auto size = draw.GetTargetSize();

    constexpr float kComboBarWidth = 80.0f;

    std::string comboText = "Combo x" + std::to_string(data->combo);
    auto sz = draw.GetTextSize(comboText, true);
    auto pos = olc::vf2d{ size.x - sz.x - gHUDPadding, sz.y + gHUDPadding };
    draw.StringProp(pos + olc::vf2d{ 1, 1 } + m_textOff, comboText, olc::Colour::BLACK);
    draw.StringProp(pos + m_textOff, comboText, olc::Colour::WHITE);
    draw.Rect(
        { size.x - (kComboBarWidth + gHUDPadding) + 1, pos.y + sz.y + 4.0f + m_textOff.y + 1 },
        { 80.0f, 5.0f },
        olc::Colour::BLACK
    );
    draw.FilledRect(
        { size.x - (kComboBarWidth + gHUDPadding), pos.y + sz.y + 4.0f + m_textOff.y },
        { 80.0f * (m_timer / gComboResetTimer), 5.0f },
        olc::Colour::TANGERINE
    );
    draw.Rect(
        { size.x - (kComboBarWidth + gHUDPadding), pos.y + sz.y + 4.0f + m_textOff.y },
        { 80.0f, 5.0f },
        olc::Colour::WHITE
    );
}
