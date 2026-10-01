#define OLC_PGE3_APPLICATION
#include "olcPixelGameEngine3.h"

#include "logic.h"
#include "modes/in_game_state.hpp"

class VolatileGame : public olc::PixelGameEngine
{
public:
	VolatileGame()
	{
		sAppName = "Volatile";
	}

public:

	bool OnUserCreate() override
	{
        ImageRepository::Get(this);

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
