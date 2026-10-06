#pragma once

#include "../logic.h"
#include "../gui.h"

class AboutState : public GameMode {
public:
    bool OnCreate(olc::PixelGameEngine* pge) override
    {
        ui = gui::State(pge);
        return false;
    }

    PlayState OnUpdate(olc::PixelGameEngine* pge, float fElapsedTime) override
    {
        static const std::string kControls[] = {
            "WASD - move",
            "Mouse - aim",
            "Left click - shoot",
            "Esc / P - pause",
        };

        // Matches the license comments in main.cpp
        static const std::string kCredits[] = {
            "pistol: morganpurkis (CC0)",
            "shotgun: hyperix6 (CC0)",
            "machine gun: qubodup (CC0)",
            "pea: MiSchy (CC0)",
            "duck: mokasza (CC BY 4.0)",
            "nerf: LaCezio (CC BY-NC 3.0)",
            "bullet hit: PotaterSalad (CC BY 4.0)",
            "punch: JohnLoser (CC BY 4.0)",
            "player death: Far_Box_creature (CC BY 4.0)",
            "weapon shuffle: Cpfcfan10 (CC BY 4.0)",
            "weapon select: InspectorJ (CC BY 4.0)",
            "hint: dland (CC0)",
            "bam: Benboncan, HerbertBoland (CC BY 4.0)",
            "combo: Scrampunk (CC BY 4.0)",
            "combo lose: DrMrSir (CC BY 4.0)",
            "heartbeat: newlocknew (CC BY-NC 4.0)",
            "ui hover, ui confirm: Feraly_ (CC0)",
            "footstep: d00121058 (CC0)",
            "enemy sounds: Diego Lopes",
        };

        constexpr int kLine = 12;
        constexpr int kMargin = 20;

        ui.Update();

        auto& draw = pge->GetDraw();
        auto size = draw.GetTargetSize();
        draw.Clear(olc::Colour::VERY_DARK_BLUE);

        PlayState next = PlayState::ABOUT;
        if (pge->GetKeyboard().GetKey(olc::Key::ESCAPE).bPressed) next = PlayState::MENU;

        gui::ContainerRect(ui, {{kMargin, kMargin}, {size.x - kMargin * 2, size.y - kMargin * 2}});
        gui::Label(ui, gui::ContainerTop(ui, 30), "ABOUT", {2.0f, 2.0f});

        auto [backPos, backSize] = gui::ContainerBottom(ui, 24);
        if (gui::Button(ui, {backPos, {100, backSize.y}}, "back", "Back")) next = PlayState::MENU;

        // Left column: controls and author
        gui::ContainerRect(ui, gui::ContainerLeft(ui, 200));
        gui::Label(ui, gui::ContainerTop(ui, kLine + 4), "CONTROLS", {1, 1}, olc::Colour::YELLOW);
        for (auto& line : kControls) gui::Label(ui, gui::ContainerTop(ui, kLine), line);
        gui::ContainerTop(ui, kLine); // gap
        gui::Label(ui, gui::ContainerTop(ui, kLine + 4), "GAME", {1, 1}, olc::Colour::YELLOW);
        gui::Label(ui, gui::ContainerTop(ui, kLine), "By Diego Lopes");
        gui::Label(ui, gui::ContainerTop(ui, kLine), "Made with olcPixelGameEngine 3");
        gui::ContainerTop(ui, kLine); // gap
        gui::Label(ui, gui::ContainerTop(ui, kLine + 4), "AI DISCLOSURE", {1, 1}, olc::Colour::YELLOW);
        gui::Label(ui, gui::ContainerTop(ui, kLine), "Code partly written with");
        gui::Label(ui, gui::ContainerTop(ui, kLine), "AI assistance (Claude Opus 5.5");
        gui::Label(ui, gui::ContainerTop(ui, kLine), "and Sonnet 5.5)");
        gui::ContainerPop(ui);

        // Right column: sound credits
        gui::Label(ui, gui::ContainerTop(ui, kLine + 4), "SOUNDS (freesound.org)", {1, 1}, olc::Colour::YELLOW);
        for (auto& line : kCredits) gui::Label(ui, gui::ContainerTop(ui, kLine), line);

        gui::ContainerPop(ui);

        return next;
    }

    gui::State ui;
};
