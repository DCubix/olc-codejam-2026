#pragma once

#include "../logic.h"
#include "../gui.h"
#include "../difficulty.hpp"

#include <iterator>

class MenuState : public GameMode {
public:
    // difficulty is the InGameState's member, so the next game uses what was picked here
    explicit MenuState(Difficulty& difficulty) : difficulty(difficulty) {}

    bool OnCreate(olc::PixelGameEngine* pge) override
    {
        ui = gui::State(pge);
        return false;
    }

    PlayState OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) override
    {
        ui.Update();

        auto& draw = pge->GetDraw();
        auto size = draw.GetTargetSize();
        draw.Clear(olc::Colour::VERY_DARK_BLUE);

        const std::string title = "SHOOT 'EM UP!";
        const olc::vf2d titleScale{4.0f, 4.0f};
        auto titleSize = draw.GetTextSize(title, true, titleScale);
        draw.StringProp({size.x / 2 - titleSize.x / 2.0f, 80.0f}, title, olc::Colour::WHITE, titleScale);

        constexpr int kWidth = 160;
        constexpr int kHeight = 4 * 24 + 3 * 4; // 4 rows, 3 gaps
        PlayState next = PlayState::MENU;

        static const std::string kDifficultyNames[] = { "EASY", "NORMAL", "HARD" };
        static_assert(std::size(kDifficultyNames) == size_t(Difficulty::COUNT));
        int diff = int(difficulty);

        gui::ContainerRect(ui, {{size.x / 2 - kWidth / 2, size.y / 2 - kHeight / 2 + 30}, {kWidth, kHeight}});
        if (gui::Button(ui, gui::ContainerTop(ui, 24), "play", "Play")) next = PlayState::IN_GAME;
        gui::ContainerTop(ui, 4); // gap

        gui::Option(ui, gui::ContainerTop(ui, 24), "difficulty", diff, kDifficultyNames, std::size(kDifficultyNames));
        difficulty = Difficulty(diff);

        gui::ContainerTop(ui, 4); // gap
        if (gui::Button(ui, gui::ContainerTop(ui, 24), "about", "About")) next = PlayState::ABOUT;
#ifndef __EMSCRIPTEN__ // a browser tab cannot be closed by the page
        gui::ContainerTop(ui, 4); // gap
        if (gui::Button(ui, gui::ContainerTop(ui, 24), "quit", "Quit")) next = PlayState::QUIT;
#endif
        gui::ContainerPop(ui);

        return next;
    }

    gui::State ui;
    Difficulty& difficulty;
};
