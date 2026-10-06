#define OLC_PGE3_APPLICATION
#include "olcPixelGameEngine3.h"

#include "logic.h"
#include "modes/in_game_state.hpp"
#include "modes/menu_state.hpp"
#include "modes/about_state.hpp"

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

        CreateImage(freezeFrame, ScreenSize());

        // uProgress: 0 = frozen frame fully visible, 1 = fully covered.
        // The frame pixelates while a grid of diamonds grows over it, staggered along the diagonal.
        transitionShader.SetVertexShaderSource(
            olc::gpu::Shader_GLSL33::VS_DefaultHeader() +
            olc::gpu::Shader_GLSL33::VS_DefaultMain()
            );
        transitionShader.SetPixelShaderSource(
            olc::gpu::Shader_GLSL33::PS_DefaultHeader() + R"(
            uniform float uProgress;
            const float kCell = 24.0;
            void main() {
                float p = smoothstep(0.0, 1.0, clamp(uProgress, 0.0, 1.0));

                float block = 1.0 + floor(p * p * 16.0);
                vec2 texel = oTex * pgeTargetSizeInPixels;
                vec2 uv = (floor(texel / block) + 0.5) * block * pgeInverseTargetSizeInPixels;
                vec3 col = texture(pgeTexture0, uv).rgb;

                vec2 frag = gl_FragCoord.xy;
                vec2 cell = floor(frag / kCell);
                vec2 local = fract(frag / kCell) - 0.5;
                float stagger = (cell.x + cell.y) * kCell / (pgeTargetSizeInPixels.x + pgeTargetSizeInPixels.y);
                float size = clamp(p * 2.0 - stagger, 0.0, 1.0);
                float covered = step(abs(local.x) + abs(local.y), size);

                pixel = vec4(col * (1.0 - covered), 1.0);
            })"
            );
        if (transitionShader.Compile() != "OK") return false;
        transitionShader.CreateUniform("uProgress");

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

        /**
         * Simple or Cute UI / UX / Interface Hover sound by Feraly_ -- https://freesound.org/s/836451/ -- License: Creative Commons 0
         */
        SRL("assets/sounds/ui-hover.wav");

        /**
         * Simple or Cute UI / UX / Interface Confirm sound by Feraly_ -- https://freesound.org/s/836450/ -- License: Creative Commons 0
         */
        SRL("assets/sounds/ui-confirm.wav");

        /**
         * FX_005_footstep_stone_l.wav by d00121058 -- https://freesound.org/s/390764/ -- License: Creative Commons 0
         */
        SRL("assets/sounds/footstep.wav");

        // My own sounds
        SRL("assets/sounds/swarmer-ouch1.wav");
        SRL("assets/sounds/swarmer-ouch2.wav");
        SRL("assets/sounds/tank-death1.wav");
        SRL("assets/sounds/tank-death2.wav");
        SRL("assets/sounds/tank-hit.wav");

		auto game = std::make_unique<InGameState>();
		modes[size_t(PlayState::MENU)] = std::make_unique<MenuState>(game->difficulty);
		modes[size_t(PlayState::ABOUT)] = std::make_unique<AboutState>();
		modes[size_t(PlayState::IN_GAME)] = std::move(game);

		for (auto& mode : modes) mode->OnCreate(this);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
        if (transition != TransitionState::NONE) {
            transitionTime += fElapsedTime;
            float progress = transitionTime / kTransitionDuration;

            if (transition == TransitionState::OUT && progress >= 1.0f) {
                // OUT done: switch modes, render one frame of the new mode and freeze it for IN
                SwitchMode();
                modes[size_t(currentState)]->OnUpdate(this, 0.0f);
                FreezeScreen();
                transition = TransitionState::IN;
                transitionTime = 0.0f;
                progress = 0.0f;
            }

            if (transition == TransitionState::IN && progress >= 1.0f) {
                transition = TransitionState::NONE;
            } else {
                DrawFrozen(transition == TransitionState::OUT ? progress : 1.0f - progress);
                return true;
            }
        }

        PlayState next = modes[size_t(currentState)]->OnUpdate(this, fElapsedTime);
        if (next == PlayState::QUIT) return false;
        if (next != currentState) {
            nextState = next;
            FreezeScreen();
            transition = TransitionState::OUT;
            transitionTime = 0.0f;
        }

        return true;
	}

    void SwitchMode()
    {
        if (currentState != PlayState::MAX_STATES)
            modes[size_t(currentState)]->OnExitMode(this);
        currentState = nextState;
        modes[size_t(currentState)]->OnEnterMode(this);
    }

    // Copies what was drawn to the screen this frame into freezeFrame
    void FreezeScreen()
    {
        draw.SetTarget(freezeFrame);
        draw.Image(GetScreen().all(), {0, 0});
        draw.SetTarget(GetScreen());
    }

    void DrawFrozen(float progress)
    {
        draw.SetShader(transitionShader);
        draw.SetShaderUniform("uProgress", progress);
        draw.Image(freezeFrame.all(), {0, 0});
        draw.ResetShader();
    }

	std::array<std::unique_ptr<GameMode>, size_t(PlayState::MAX_STATES)> modes;
	PlayState currentState{PlayState::MAX_STATES}; // no mode entered yet
    PlayState nextState{PlayState::MENU};

    ext::Miniaudio::AudioEngine audio;

    static constexpr float kTransitionDuration = 0.6f; // seconds, per half (OUT and IN)
    enum class TransitionState { NONE, OUT, IN };
    // Starts as a finished OUT, so the first frame enters nextState and plays IN
    TransitionState transition{TransitionState::OUT};
    float transitionTime{kTransitionDuration};

    olc::Image freezeFrame;
    olc::gpu::Shader_GLSL33 transitionShader;
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
