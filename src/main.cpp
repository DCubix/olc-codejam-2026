#define OLC_PGE3_APPLICATION
#include "olcPixelGameEngine3.h"

#include "logic.h"
#include "modes/in_game_state.hpp"

#include "olcPGEX3_Miniaudio.h"

class VolatileGame : public olc::PixelGameEngine
{
public:
	VolatileGame()
	{
		sAppName = "Volatile";
        audio.EnableBackgroundPlayback();
        InstallSystemExtension(&audio);
	}

public:

	bool OnUserCreate() override
	{
        srand(unsigned(time(nullptr)));
        ImageRepository::Get(this);
        SoundRepository::Get(&audio);

        ImageRepository::Get().LoadImage("assets/sprites/gun1.png", {-2, 17});
        ImageRepository::Get().LoadImage("assets/sprites/gun2.png", {-2, 17});
        ImageRepository::Get().LoadImage("assets/sprites/gun3.png", {-2, 17});
        ImageRepository::Get().LoadImage("assets/sprites/gun4.png", {-2, 17});
        ImageRepository::Get().LoadImage("assets/sprites/gun5.png", {-2, 17});
        ImageRepository::Get().LoadImage("assets/sprites/gun6.png", {-2, 17});

        ImageRepository::Get().LoadImage("assets/sprites/bullet1.png", {4, 2});
        ImageRepository::Get().LoadImage("assets/sprites/bullet2.png", {2, 1});
        ImageRepository::Get().LoadImage("assets/sprites/bullet3.png", {5, 5});
        ImageRepository::Get().LoadImage("assets/sprites/bullet4.png", {15, 10});
        ImageRepository::Get().LoadImage("assets/sprites/bullet5.png", {11, 2});

        /**
         * Single Pistol Gunshot 4.wav by morganpurkis -- https://freesound.org/s/391328/ -- License: Creative Commons 0
         */
        SRL("assets/sounds/pistol.wav");

        /**
         * Shotgun Fire by hyperix6 -- https://freesound.org/s/660299/ -- License: Creative Commons 0
         */
        SRL("assets/sounds/shotgun.wav");

        /**
         * M240 Machine Gun Single Shot by qubodup -- https://freesound.org/s/854641/ -- License: Creative Commons 0
         */
        SRL("assets/sounds/machine-gun.wav");

        /**
         * Plop_2(hi).wav by MiSchy -- https://freesound.org/s/369959/ -- License: Creative Commons 0
         */
        SRL("assets/sounds/pea.wav");

        /**
         * Rapid duck quack by mokasza -- https://freesound.org/s/810326/ -- License: Attribution 4.0
         */
        SRL("assets/sounds/duck.wav");

        /**
         * Toy gun shot by LaCezio -- https://freesound.org/s/320495/ -- License: Attribution NonCommercial 3.0
         */
        SRL("assets/sounds/nerf.wav");

        /**
         * Sub Sonic Bullet Body Hit by PotaterSalad -- https://freesound.org/s/812178/ -- License: Attribution 4.0
         */
        SRL("assets/sounds/bullet-hit.wav");

        /**
         * Cyber Punch 01 by JohnLoser -- https://freesound.org/s/573376/ -- License: Attribution 4.0
         */
        SRL("assets/sounds/punch.wav");

        /**
         * LandingDamage1.wav by Far_Box_creature -- https://freesound.org/s/469564/ -- License: Attribution 4.0
         */
        SRL("assets/sounds/player-death.wav");

        /**
         * Generic Metallic Click 3 by Cpfcfan10 -- https://freesound.org/s/797644/ -- License: Attribution 4.0 (derived)
         */
        SRL("assets/sounds/weapon-shuffle.wav");

        /**
         * UI, Mechanical, Select, 01, FX.wav by InspectorJ -- https://freesound.org/s/458585/ -- License: Attribution 4.0
         */
        SRL("assets/sounds/weapon-select.wav");

        /**
         * hint.wav by dland -- https://freesound.org/s/320181/ -- License: Creative Commons 0
         */
        SRL("assets/sounds/hint.wav");

        /**
         * Inceptiomatic.wav by Benboncan -- https://freesound.org/s/104675/ -- License: Attribution 4.0
         * LargeStuckDoorHallway.wav by HerbertBoland -- https://freesound.org/s/43552/ -- License: Attribution 4.0
         */
        SRL("assets/sounds/bam.wav");

        /**
         * Itemize by Scrampunk -- https://freesound.org/s/345297/ -- License: Attribution 4.0
         */
        SRL("assets/sounds/combo.wav");

        /**
         * Noise_Shutdown.wav by DrMrSir -- https://freesound.org/s/529553/ -- License: Attribution 4.0
         */
        SRL("assets/sounds/combo-lose.wav");

        /**
         * Heart beat.Valve move and muscle work.Sound design_EM.wav by newlocknew -- https://freesound.org/s/614938/ -- License: Attribution NonCommercial 4.0
         */
        SRL("assets/sounds/heartbeat.wav");

        // My own sounds
        SRL("assets/sounds/swarmer-ouch1.wav");
        SRL("assets/sounds/swarmer-ouch2.wav");
        SRL("assets/sounds/tank-death1.wav");
        SRL("assets/sounds/tank-death2.wav");
        SRL("assets/sounds/tank-hit.wav");

		modes[size_t(PlayState::IN_GAME)] = std::make_unique<InGameState>();

		for (auto& mode : modes) mode->OnCreate(this);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		auto& mode = modes[size_t(currentState)];
		
		if (prevState != currentState) {
			if (prevState != PlayState::MAX_STATES)
				modes[size_t(prevState)]->OnExitMode(this);
			mode->OnEnterMode(this);
			prevState = currentState;
		}

		currentState = mode->OnUpdate(this, fElapsedTime);

        return true;
	}

	std::array<std::unique_ptr<GameMode>, size_t(PlayState::MAX_STATES)> modes;
	PlayState currentState{PlayState::IN_GAME};
	PlayState prevState{PlayState::MAX_STATES};

    ext::Miniaudio::AudioEngine audio;
};

int main()
{
	VolatileGame demo;

	PGEConfig config;
	config.bVSync = false;
	config.vPixelSize = { 2, 2 };
	config.vScreenSize = { 640,360 };

	if (demo.Construct(config))
	{
		demo.Start();
	}

	return 0;
}
