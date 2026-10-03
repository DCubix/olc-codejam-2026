#pragma once

#include <cstddef>

// Gameplay parameters that change with difficulty.
// The Normal preset holds the values currently hardcoded in the game.
// Nothing reads this file yet.

enum class Difficulty { EASY = 0, NORMAL, HARD, COUNT };

struct DifficultyParams {
    // Player (player.h, player.cpp)
    int   playerMaxHealth;          // gPlayerMaxHealth
    int   playerCriticalHealth;     // gCriticalPlayerHealth, red vignette starts at or below this
    float playerMoveSpeed;          // units/s, base speed in Player::OnUpdate
    float playerRegenInterval;      // seconds without damage per +1 health
    int   damageHitsToLoseCombo;    // hits taken during a combo before it resets

    // Combo (logic.h, combodisplay.h, in_game_state.hpp AwardScore)
    int   comboEveryNHits;          // gComboEveryNHits
    float comboResetTimer;          // gComboResetTimer, seconds before the combo expires
    int   fireRateStepPerCombo;     // gFireRateStepUpPerCombo
    int   maxFireRateMultiplier;    // gMaxFireRateMultiplier
    int   moveSpeedStepPerCombo;    // gMoveSpeedMultiplierStepUpPerCombo
    int   maxMoveSpeedMultiplier;   // clamp in AwardScore
    int   scoreStepPerCombo;        // gScoreMultiplierStepUpPerCombo
    int   maxScoreMultiplier;       // clamp in AwardScore

    // Weapon shuffler (weaponshuffler.h)
    float swapCountdownMin;         // gSwapCountdownMin
    float swapCountdownMax;         // gSwapCountdownMax
    float shuffleLeadTime;          // gShuffleLeadTime
    float shuffleHoldTime;          // m_holdTime

    // Enemy AI (logic.h)
    float chaseDistance;            // gMinDistanceChase
    float attackDistance;           // gMinDistanceAttack

    // Swarmer (swarmer.cpp)
    float swarmerSpeed;
    int   swarmerDamage;
    int   swarmerScore;             // AwardScore default amount

    // Tank (tank.h, tank.cpp)
    float tankSpeed;
    int   tankDamage;
    int   tankHealth;               // dies when a hit is taken at 0, so hits to kill = health + 1
    int   tankScore;

    // Spawner (in_game_state.hpp OnUpdate)
    int   maxSwarmers;              // spawning pauses at this many swarmers
    float spawnInterval;            // seconds between spawns
    float tankChance;               // 0..1 chance a spawn is a tank
    float spawnDistanceX;           // spawn offset from the player, left or right
    float spawnRangeY;              // spawn offset from the player, +/- vertical
};

inline constexpr DifficultyParams gDifficulties[] = {
    // EASY
    {
        .playerMaxHealth = 1500,
        .playerCriticalHealth = 300,
        .playerMoveSpeed = 150.0f,
        .playerRegenInterval = 0.5f,
        .damageHitsToLoseCombo = 5,

        .comboEveryNHits = 2,
        .comboResetTimer = 4.0f,
        .fireRateStepPerCombo = 6,
        .maxFireRateMultiplier = 70,
        .moveSpeedStepPerCombo = 2,
        .maxMoveSpeedMultiplier = 100,
        .scoreStepPerCombo = 1,
        .maxScoreMultiplier = 100,

        .swapCountdownMin = 20.0f,
        .swapCountdownMax = 30.0f,
        .shuffleLeadTime = 6.0f,
        .shuffleHoldTime = 3.0f,

        .chaseDistance = 400.0f,
        .attackDistance = 40.0f,

        .swarmerSpeed = 50.0f,
        .swarmerDamage = 1,
        .swarmerScore = 5,

        .tankSpeed = 30.0f,
        .tankDamage = 1,
        .tankHealth = 2,
        .tankScore = 10,

        .maxSwarmers = 30,
        .spawnInterval = 0.8f,
        .tankChance = 0.05f,
        .spawnDistanceX = 600.0f,
        .spawnRangeY = 400.0f,
    },
    // NORMAL
    {
        .playerMaxHealth = 1000,
        .playerCriticalHealth = 250,
        .playerMoveSpeed = 140.0f,
        .playerRegenInterval = 1.0f,
        .damageHitsToLoseCombo = 3,

        .comboEveryNHits = 3,
        .comboResetTimer = 3.0f,
        .fireRateStepPerCombo = 5,
        .maxFireRateMultiplier = 70,
        .moveSpeedStepPerCombo = 2,
        .maxMoveSpeedMultiplier = 100,
        .scoreStepPerCombo = 2,
        .maxScoreMultiplier = 100,

        .swapCountdownMin = 15.0f,
        .swapCountdownMax = 25.0f,
        .shuffleLeadTime = 6.0f,
        .shuffleHoldTime = 3.0f,

        .chaseDistance = 500.0f,
        .attackDistance = 40.0f,

        .swarmerSpeed = 60.0f,
        .swarmerDamage = 1,
        .swarmerScore = 5,

        .tankSpeed = 40.0f,
        .tankDamage = 1,
        .tankHealth = 3,
        .tankScore = 10,

        .maxSwarmers = 50,
        .spawnInterval = 0.4f,
        .tankChance = 0.15f,
        .spawnDistanceX = 600.0f,
        .spawnRangeY = 400.0f,
    },
    // HARD
    {
        .playerMaxHealth = 600,
        .playerCriticalHealth = 200,
        .playerMoveSpeed = 130.0f,
        .playerRegenInterval = 2.0f,
        .damageHitsToLoseCombo = 2,

        .comboEveryNHits = 4,
        .comboResetTimer = 2.0f,
        .fireRateStepPerCombo = 4,
        .maxFireRateMultiplier = 60,
        .moveSpeedStepPerCombo = 2,
        .maxMoveSpeedMultiplier = 100,
        .scoreStepPerCombo = 3,
        .maxScoreMultiplier = 100,

        .swapCountdownMin = 10.0f,
        .swapCountdownMax = 20.0f,
        .shuffleLeadTime = 5.0f,
        .shuffleHoldTime = 2.0f,

        .chaseDistance = 650.0f,
        .attackDistance = 40.0f,

        .swarmerSpeed = 75.0f,
        .swarmerDamage = 2,
        .swarmerScore = 5,

        .tankSpeed = 50.0f,
        .tankDamage = 3,
        .tankHealth = 5,
        .tankScore = 15,

        .maxSwarmers = 70,
        .spawnInterval = 0.25f,
        .tankChance = 0.25f,
        .spawnDistanceX = 600.0f,
        .spawnRangeY = 400.0f,
    },
};

static_assert(sizeof(gDifficulties) / sizeof(gDifficulties[0]) == size_t(Difficulty::COUNT),
              "one preset per Difficulty value");

inline constexpr const DifficultyParams& GetDifficulty(Difficulty d) {
    return gDifficulties[size_t(d)];
}
