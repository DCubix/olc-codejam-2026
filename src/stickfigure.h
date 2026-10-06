#pragma once

#include "olcPixelGameEngine3.h"

#include "array.hpp"
#include <map>

constexpr size_t gMaxSticksPerFigure = 64;

using StickID = int32_t;

enum class StickShape { NONE, CAPSULE, CIRCLE, SPRITE };

struct Sprite {
    olc::Image* image{ nullptr };
    olc::vi2d origin{ 0, 0 };
    int tilesX{ 1 }, tilesY{ 1 };
};

struct Stick {
    StickID id{ -1 };
    StickShape shape;
    olc::vf2d offset;
    float rotation, length, width;
    olc::Pixel color;

    // Sprite stick
    Sprite sprite{};
    uint32_t frame{ 0 };

    // animated values
    olc::vf2d animatedOffset{0,0};
    float animatedRotation{0.0f};
};

struct Keyframe {
    olc::vf2d offset;
    float rotation;
    int spriteFrame{0};
};

using Track = std::map<uint32_t, Keyframe>;

enum class AnimationMode { ONE_SHOT, LOOP, PING_PONG };

struct Animation {
    uint32_t currentFrame{0};
    uint32_t durationFrames;
    AnimationMode mode;
    float timer{0.0f};
    bool pingPongForward{true};
};

class Figure {
public:
    StickID Add(
        float length, float rotation, float width,
        olc::Pixel color,
        StickShape shape = StickShape::CAPSULE,
        olc::vf2d offset = olc::vf2d{ 0.0f, 0.0f },
        StickID parent = -1
    );
    void ParseString(const std::string& stkSource, const std::filesystem::path& baseDir = {});
    void ParseFile(const std::string& fileName);

    Stick* GetStick(StickID sid);
    StickID GetStickID(const std::string& name);
    olc::vf2d GetStickWorldOffset(StickID sid) const;
    olc::vf2d GetStickWorldTipOffset(StickID sid) const;

    void Update(float fElapsedTime);
    void Draw(
        olc::Draw& draw,
        bool flipX = false,
        std::optional<olc::Pixel> colorOverride = std::nullopt,
        std::optional<olc::Pixel> light = std::nullopt
    );

    void PlayAnimation(const std::string& name, float timeScale = 1.0f);
    bool IsAnimationFinished(const std::string& name) const;
    uint32_t GetCurrentFrame(const std::string& name) const;

    olc::vf2d Size() const { return m_size; }

private:
    Array<Stick, gMaxSticksPerFigure> m_sticks{};
    Array<int32_t, gMaxSticksPerFigure> m_stickOrdering{};
    Array<StickID, gMaxSticksPerFigure> m_parents{};
    std::map<std::string, StickID> m_stickNames;
    StickID m_stickId{ 0 };
    olc::vf2d m_size{};

    // animation
    std::map<std::string, Animation> m_animations;
    std::map<std::string, std::array<Track, gMaxSticksPerFigure>> m_animtationTracks;

    std::string m_currentAnimation{};
    float m_animationTimeScale{1.0f};

    olc::vf2d GetStickTipOffset(StickID sid) const;
    float GetStickWorldRotation(StickID sid) const;
    void UpdateSize();

    void DrawStick(
        olc::Draw& draw,
        StickID sid,
        std::optional<olc::Pixel> colorOverride = std::nullopt,
        std::optional<olc::Pixel> light = std::nullopt
    );
};
