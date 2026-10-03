#include "statsdisplay.h"

constexpr float gWeaponDisplaySize = 48.0f;

StatsDisplay::StatsDisplay()
{
    Show();
}

void StatsDisplay::OnUpdate(olc::PixelGameEngine *pge, float fElapsedTime)
{
    auto& draw = pge->GetDraw();
    auto size = pge->ScreenSize();

    auto fnDrawBar = [&](olc::vf2d pos, olc::vf2d size, float value, olc::Pixel color, const std::string& label) {
        draw.FilledRect(pos, size, olc::PixelF(0.0f, 0.0f, 0.0f, 0.5f));
        draw.FilledRect(pos, olc::vf2d{size.x * value, size.y}, color);
        draw.Rect(pos, size, olc::Colour::WHITE);

        if (!label.empty()) {
            auto labelColor = color * 0.5f;
            draw.StringProp(pos + olc::vf2d{8.0f, size.y / 2.0f - 4.0f}, label, labelColor);
        }
    };

    auto fnLeftPad = [](const std::string& str, size_t width, char c = ' ') {
        if (str.length() >= width) return str;
        return std::string(width - str.length(), c) + str;
    };

    constexpr float kHealthBarHeight = 13.0f;
    constexpr float kFireBarHeight = 7.0f;

    const auto& w = gWeapons[player->weapon];

    // Draw weapon display
    olc::vf2d frameSize{gWeaponDisplaySize, gWeaponDisplaySize};
    auto framePos = olc::vf2d{ gHUDPadding, size.y - frameSize.y - gHUDPadding };
    draw.FilledRect(framePos, frameSize, olc::PixelF(0.0f, 0.0f, 0.0f, 0.5f));
    draw.Rect(framePos, frameSize, olc::Colour::WHITE);

    const float weaponSpriteSize = gWeaponDisplaySize * 0.8f;
    olc::vf2d weaponSize{weaponSpriteSize, weaponSpriteSize};
    auto weaponPos = framePos + olc::vf2d{ frameSize.x / 2.0f - weaponSize.x / 2.0f, frameSize.y / 2.0f - weaponSize.y / 2.0f };
    draw.Image(w.equipAsset->image->all(), weaponPos, weaponSpriteSize / w.equipAsset->image->Size());

    // Draw health bar
    olc::vf2d healthBarPos{ framePos.x + frameSize.x + 4.0f, framePos.y };
    float healthValue = float(player->health) / params->playerMaxHealth;
    fnDrawBar(healthBarPos, olc::vf2d{120.0f, kHealthBarHeight}, healthValue, olc::Colour::RED, "Health");

    // Draw shoot timer
    olc::vf2d shootBarPos{ healthBarPos.x, healthBarPos.y + kHealthBarHeight + 4.0f };

    float fireRateMult = float(100 - data->fireRateMultiplier) / 100.0f;
    float fireRate = w.fireRate * fireRateMult;
    float shootValue = std::min(1.0f, player->shootTimer / fireRate);

    fnDrawBar(shootBarPos, olc::vf2d{120.0f, kFireBarHeight}, shootValue, olc::Colour::TANGERINE, "");

    // Draw score
    constexpr int kMaxScoreDigits = 10;
    const olc::vf2d textScale = olc::vf2d{1.0f, 1.0f} * 1.8f;
    std::string scoreText = std::to_string(data->score);
    std::string paddedScoreText = fnLeftPad(scoreText, kMaxScoreDigits);
    if (scoreText.length() < kMaxScoreDigits) {
        std::string zeros = std::string(kMaxScoreDigits-scoreText.length(), '0') + std::string(scoreText.length(), ' ');
        draw.String(olc::vf2d{ healthBarPos.x, shootBarPos.y + kFireBarHeight + 4.0f }, zeros, olc::Colour::YELLOW * 0.4f, textScale);
    }
    draw.String(olc::vf2d{ healthBarPos.x, shootBarPos.y + kFireBarHeight + 4.0f }, paddedScoreText, olc::Colour::YELLOW, textScale);
}
