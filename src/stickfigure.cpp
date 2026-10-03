#include "stickfigure.h"
#include "scanner.hpp"
#include "utils.hpp"
#include "repository.h"

#include <stack>
#include <ranges>
#include <fstream>
#include <numbers>
#include <limits>

#include <inttypes.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

StickID Figure::Add(
    float length, float rotation, float width,
    olc::Pixel color,
    StickShape shape,
    olc::vf2d offset,
    StickID parent
) {
    StickID newId = m_stickId++;
    auto &stk = m_sticks.AddInPlace();
    stk.id = newId;
    stk.length = length;
    stk.rotation = rotation;
    stk.width = width;
    stk.offset = offset;
    stk.shape = shape;
    stk.color = color;

    m_parents.Add(parent);
    return newId;
}

Stick* Figure::GetStick(StickID sid)
{
    if (sid < 0 || sid >= m_sticks.Size()) return nullptr;
    return &m_sticks[sid];
}

olc::vf2d Figure::GetStickWorldOffset(StickID sid) const
{
    const auto& stick = m_sticks[sid];
    const auto localOffset = stick.offset + stick.animatedOffset;
    const auto parent = m_parents[sid];
    if (parent < 0) return localOffset;

    const auto parentOffset = GetStickWorldTipOffset(parent);
    const float parentRotation = GetStickWorldRotation(parent);
    const float c = std::cos(parentRotation), s = std::sin(parentRotation);
    const olc::vf2d rotatedOffset{
        c * localOffset.x - s * localOffset.y,
        s * localOffset.x + c * localOffset.y
    };
    return parentOffset + rotatedOffset;
}

olc::vf2d Figure::GetStickTipOffset(StickID sid) const
{
    const auto& stick = m_sticks[sid];
    const float rotation = GetStickWorldRotation(sid);
    const float c = std::cos(rotation), s = std::sin(rotation);
    return olc::vf2d{
        c * stick.length, s * stick.length
    } + stick.offset + stick.animatedOffset;
}

olc::vf2d Figure::GetStickWorldTipOffset(StickID sid) const
{
    return GetStickWorldOffset(sid) + (GetStickTipOffset(sid) - m_sticks[sid].offset - m_sticks[sid].animatedOffset);
}

float Figure::GetStickWorldRotation(StickID sid) const
{
    float parentRot = 0.0f;
    const auto parent = m_parents[sid];
    if (parent >= 0) {
        parentRot = GetStickWorldRotation(parent);
    }
    return parentRot + m_sticks[sid].rotation + m_sticks[sid].animatedRotation;
}

static void DrawCapsule(olc::Draw &draw, olc::vf2d p1, olc::vf2d p2, float width, const olc::Pixel& color) {
    const float hw = width / 2.0f;

    auto n = (p2 - p1).norm().perp() * hw;

    std::vector<olc::vf2d> points{ p1 + n, p2 + n, p2 - n, p1 - n };

    draw.FilledPolygon(olc::Structure::Fan, points, color);
    draw.FilledCircle(p1, hw, color);
    draw.FilledCircle(p2, hw, color);
}

void Figure::DrawStick(olc::Draw &draw, StickID sid, std::optional<olc::Pixel> colorOverride, std::optional<olc::Pixel> light)
{
    const auto& stick = m_sticks[sid];
    if (stick.shape == StickShape::NONE) return;

    auto color = colorOverride
        ? *colorOverride
        : stick.color;


    auto li = light.value_or(olc::Colour::WHITE);
    color.r = MulU8(color.r, li.r);
    color.g = MulU8(color.g, li.g);
    color.b = MulU8(color.b, li.b);
    
    float luma = (float(color.r) + float(color.g) + float(color.b)) / 765.0f;
    // const auto outlineColor = luma < 0.2f
    //     ? olc::Colour::WHITE : olc::Colour::BLACK;
    const auto outlineColor = olc::Colour::BLACK;

    switch (stick.shape) {
        case StickShape::CAPSULE: {
            auto p1 = GetStickWorldOffset(sid).round();
            auto p2 = GetStickWorldTipOffset(sid).round();
            DrawCapsule(draw, p1, p2, stick.width+2.5f, outlineColor);
            DrawCapsule(draw, p1, p2, stick.width, color);
        } break;
        case StickShape::CIRCLE: {
            auto p1 = GetStickWorldOffset(sid).round();
            auto rot = GetStickWorldRotation(sid);
            auto tmp = draw.GetWorldTransform();

            draw.WorldOffset(p1);
            draw.WorldRotate(rot);
            draw.FilledCircle({ stick.length / 2.0f, 0.0f }, stick.length / 2.0f + 1.25f, outlineColor);
            draw.FilledCircle({ stick.length / 2.0f, 0.0f }, stick.length / 2.0f, color);

            draw.SetWorldTransform(tmp);
        } break;
        case StickShape::SPRITE: {
            if (stick.sprite.image == nullptr) break;

            auto p1 = GetStickWorldOffset(sid).round();
            auto rot = GetStickWorldRotation(sid);

            const olc::vf2d tileSize = {
                float(stick.sprite.image->Size().x / stick.sprite.tilesX),
                float(stick.sprite.image->Size().y / stick.sprite.tilesY)
            };

            // scale it based on stick.length, keeping the aspect ratio
            float scale = stick.length / tileSize.x;

            auto tmp = draw.GetWorldTransform();

            draw.WorldOffset(p1);
            draw.WorldRotate(rot);

            // region() takes source pixel coordinates
            const uint32_t frame = stick.frame % uint32_t(stick.sprite.tilesX * stick.sprite.tilesY);
            olc::vf2d src = {
                float(frame % stick.sprite.tilesX) * tileSize.x,
                float(frame / stick.sprite.tilesX) * tileSize.y
            };
            auto region = stick.sprite.image->region(src, tileSize);

            draw.Image(region, -olc::vf2d(stick.sprite.origin) * scale, { scale, scale }, color);

            draw.SetWorldTransform(tmp);
        } break;
        default: break;
    }
}

void Figure::Draw(
    olc::Draw &draw,
    float fElapsedTime,
    bool flipX,
    std::optional<olc::Pixel> colorOverride,
    std::optional<olc::Pixel> light,
    std::function<void()> preDraw
)
{
    auto fnLerpAngle = [](float start, float end, float t) {
        constexpr float PI = std::numbers::pi_v<float>;
        constexpr float TWO_PI = PI * 2.0f;
        float delta = std::fmod(end - start, TWO_PI);
        if (delta > PI) delta -= TWO_PI;
        else if (delta < -PI) delta += TWO_PI;
        return start + delta * t;
    };

    auto fnAnimateStick = [=](Stick& stk, const Track& track, uint32_t frameNo) {
        if (track.empty()) return;

        auto last = std::prev(track.end());
        auto kf0 = track.begin();

        if (frameNo <= kf0->first) {
            stk.animatedOffset = kf0->second.offset;
            stk.animatedRotation = kf0->second.rotation;
            stk.frame = kf0->second.spriteFrame;
            return;
        }

        if (frameNo >= last->first) {
            stk.animatedOffset = last->second.offset;
            stk.animatedRotation = last->second.rotation;
            stk.frame = last->second.spriteFrame;
            return;
        }

        auto k2 = track.upper_bound(frameNo);
        auto k1 = std::prev(k2);

        const auto t1 = k1->first;
        const auto t2 = k2->first;
        const float t = float(frameNo - t1) / float(t2 - t1);

        stk.animatedOffset = k1->second.offset.lerp(k2->second.offset, t);
        stk.animatedRotation = fnLerpAngle(
            k1->second.rotation,
            k2->second.rotation,
            t
        );
        // sprite frame: linear interpolation, rounded down to a whole tile index
        const float f1 = float(k1->second.spriteFrame), f2 = float(k2->second.spriteFrame);
        stk.frame = uint32_t(std::max(0.0f, std::floor(f1 + (f2 - f1) * t)));
    };

    if (!m_currentAnimation.empty()) {
        auto& anim = m_animations[m_currentAnimation];

        const float timeStep = 1.0f / 60.0f;

        // update animation timing
        anim.timer += fElapsedTime * m_animationTimeScale;
        if (anim.durationFrames == 0) {
            anim.currentFrame = 0;
        } else if (anim.timer >= timeStep) {
            anim.timer -= timeStep;
            switch (anim.mode) {
                case AnimationMode::ONE_SHOT: {
                    if (++anim.currentFrame >= anim.durationFrames) {
                        anim.currentFrame = anim.durationFrames-1;
                    }
                } break;
                case AnimationMode::LOOP: {
                    if (++anim.currentFrame >= anim.durationFrames) {
                        anim.currentFrame = 0;
                    }
                } break;
                case AnimationMode::PING_PONG: {
                    if (anim.durationFrames <= 1) {
                        anim.currentFrame = 0;
                        break;
                    }

                    if (anim.pingPongForward) {
                        if (anim.currentFrame + 1 >= anim.durationFrames) {
                            anim.currentFrame = anim.durationFrames - 1;
                            anim.pingPongForward = false;
                        } else {
                            ++anim.currentFrame;
                        }
                    } else {
                        if (anim.currentFrame == 0) {
                            anim.pingPongForward = true;
                            ++anim.currentFrame;
                        } else {
                            --anim.currentFrame;
                        }
                    }
                } break;
            }
        }

        // update sticks
        const auto& tracks = m_animtationTracks[m_currentAnimation];
        for (std::size_t sid = 0; sid < tracks.size(); ++sid) {
            const auto& track = tracks[sid];

            if (track.empty()) continue;

            if (auto* stick = GetStick(static_cast<StickID>(sid))) {
                fnAnimateStick(*stick, track, anim.currentFrame);
            }
        }
    }

    if (preDraw) preDraw();

    std::vector<StickID> orderedSticks;
    orderedSticks.reserve(m_sticks.Size());
    for (StickID sid = 0; sid < m_sticks.Size(); sid++) {
        orderedSticks.push_back(sid);
    }

    std::stable_sort(orderedSticks.begin(), orderedSticks.end(),
        [this](StickID lhs, StickID rhs) {
            return m_stickOrdering[lhs] < m_stickOrdering[rhs];
        });

    auto savedTransform = draw.GetWorldTransform();
    if (flipX) draw.WorldScale({ -1.0f, 1.0f });

    for (const auto sid : orderedSticks) {
        DrawStick(draw, sid, colorOverride, light);
    }

    draw.SetWorldTransform(savedTransform);
}

void Figure::PlayAnimation(const std::string &name, float timeScale)
{
    auto pos = m_animations.find(name);
    if (pos == m_animations.end()) return;
    if (m_currentAnimation == name) return;

    m_currentAnimation = name;
    m_animationTimeScale = timeScale;
    m_animations[name].currentFrame = 0;
    m_animations[name].timer = 0;
    m_animations[name].pingPongForward = true;
}

bool Figure::IsAnimationFinished(const std::string &name) const
{
    auto pos = m_animations.find(name);
    if (pos == m_animations.end()) return false;

    const auto& anim = pos->second;
    if (anim.mode != AnimationMode::ONE_SHOT) return false;
    if (anim.durationFrames == 0) return true;

    return anim.currentFrame + 1 >= anim.durationFrames;
}

uint32_t Figure::GetCurrentFrame(const std::string &name) const
{
    auto pos = m_animations.find(name);
    if (pos == m_animations.end()) return false;

    const auto& anim = pos->second;
    return anim.currentFrame;
}

void Figure::UpdateSize()
{
    olc::vf2d mn{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
    olc::vf2d mx{ std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };
    bool any = false;

    for (StickID sid = 0; sid < m_sticks.Size(); sid++) {
        const auto& stick = m_sticks[sid];
        if (stick.shape == StickShape::NONE) continue;

        any = true;
        olc::vf2d center;
        float radius;

        if (stick.shape == StickShape::CAPSULE) {
            const auto p1 = GetStickWorldOffset(sid);
            const auto p2 = GetStickWorldTipOffset(sid);
            radius = stick.width / 2.0f;

            mn.x = std::min({ mn.x, p1.x - radius, p2.x - radius });
            mn.y = std::min({ mn.y, p1.y - radius, p2.y - radius });
            mx.x = std::max({ mx.x, p1.x + radius, p2.x + radius });
            mx.y = std::max({ mx.y, p1.y + radius, p2.y + radius });
            continue;
        }

        // CIRCLE: drawn centered at world offset + length/2 along its rotation
        radius = stick.length / 2.0f;
        const auto p1 = GetStickWorldOffset(sid);
        const auto rot = GetStickWorldRotation(sid);
        center = p1 + olc::vf2d{ std::cos(rot) * radius, std::sin(rot) * radius };

        mn.x = std::min(mn.x, center.x - radius);
        mn.y = std::min(mn.y, center.y - radius);
        mx.x = std::max(mx.x, center.x + radius);
        mx.y = std::max(mx.y, center.y + radius);
    }

    m_size = any ? (mx - mn) : olc::vf2d{};
}

std::string_view Trim(std::string_view s) {
    auto is_space = [](unsigned char c) { return std::isspace(c); };

    auto start = std::ranges::find_if_not(s, is_space);
    auto end = std::ranges::find_if_not(s | std::views::reverse, is_space).base();

    return (start < end) ? std::string_view(start, end) : std::string_view();
}

template <typename T>
static T TokenOpt(
    Scanner<char>& sc,
    T defaultValue = T{},
    std::function<T(Scanner<char>&)> read = [](Scanner<char>& sc) { return sc.Token<T>(); }
) {
    sc.SkipWhitespace();
    if (sc.Peek() == '_') {
        sc.Read();
        return defaultValue;
    }
    return read(sc);
};

void Figure::ParseString(const std::string& stkSource, const std::filesystem::path& baseDir)
{
    m_stickId = 0;
    m_stickNames.clear();
    m_sticks.Clear();
    m_parents.Clear();
    m_animations.clear();
    m_animtationTracks.clear();
    m_stickOrdering.Clear();

    olc::Pixel fallbackColor = olc::Colour::BLACK;
    std::stack<StickID> rootStack;

    std::string currentAnimation = "";

    auto fnReadColor = [](Scanner<char>& sc) -> olc::Pixel {
        uint8_t r, g, b;

        if (sc.Peek() == '#') { // hex string
            std::string comp = TokenOpt<std::string>(sc, "#000000");
            std::sscanf(comp.c_str(), "#%" SCNu8 "%" SCNu8 "%" SCNu8, &r, &g, &b);
        } else {
            r = uint8_t(TokenOpt<int>(sc) & 0xFF);
            if (sc.IsEOF()) return olc::Colour::BLACK;

            g = uint8_t(TokenOpt<int>(sc) & 0xFF);
            if (sc.IsEOF()) return olc::Colour::BLACK;

            b = uint8_t(TokenOpt<int>(sc) & 0xFF);
        }
        return olc::Pixel(r, g, b);
    };

    auto fnReadAnimationMode = [](Scanner<char>& sc) -> AnimationMode {
        auto tok = TokenOpt<std::string>(sc, "0");
        std::transform(tok.begin(), tok.end(), tok.begin(), ::tolower);

        if (tok == "l" || tok == "1") return AnimationMode::LOOP;
        else if (tok == "p" || tok == "2") return AnimationMode::PING_PONG;
        return AnimationMode::ONE_SHOT;
    };

    auto fnUpdateParentStack = [&](Scanner<char>& sc, StickID sid) {
        if (!sc.IsEOF() && sc.Peek() == '=') {
            sc.Read();
            sc.SkipWhitespace();
            auto tag = sc.Token<std::string>([](char c) { return ::isspace(c) || c == '>' || c == '<'; });
            m_stickNames[tag] = sid;
        }

        sc.SkipWhitespace();

        if (!sc.IsEOF() && sc.Peek() == '>') {
            sc.Read();
            rootStack.push(sid);
        } else if (!sc.IsEOF() && sc.Peek() == '<') {
            sc.Read();
            if (!rootStack.empty()) rootStack.pop();
        }
    };

    for (auto line : stkSource | std::views::split('\n')) {
        std::string sv(line.begin(), line.end());
        sv = Trim(sv);

        if (sv.empty() || sv[0] == '#') continue;
        
        Scanner sc(sv);
        const auto cmd = sc.Token<std::string>();
        sc.SkipWhitespace();

        switch (cmd[0]) {
            case 'p':
            case 'P': fallbackColor = fnReadColor(sc); break;
            case 'c':
            case 'C': { // capsule stick
                olc::Pixel color = fallbackColor;

                if (sc.IsEOF()) continue;

                float x = sc.Token<float>();
                float y = sc.Token<float>();
                float len = sc.Token<float>();
                float rot = Deg2Rad(TokenOpt<float>(sc));
                float width = sc.Token<float>();

                sc.SkipWhitespace();

                if (!sc.IsEOF() && sc.Peek() != '=') {
                    color = fnReadColor(sc);
                }

                sc.SkipWhitespace();

                int32_t order = 0;
                if (!sc.IsEOF() && sc.Peek() != '=') {
                    order = TokenOpt<int32_t>(sc);
                }

                sc.SkipWhitespace();

                auto parent = rootStack.empty() ? -1 : rootStack.top();
                auto sid = Add(len, rot, width, color, StickShape::CAPSULE, {x, y}, parent);
                m_stickOrdering.Add(order);

                fnUpdateParentStack(sc, sid);
            } break;
            case 'o':
            case 'O': {
                olc::Pixel color = fallbackColor;

                if (sc.IsEOF()) continue;

                float x = sc.Token<float>();
                float y = sc.Token<float>();
                float diam = sc.Token<float>();
                float rot = Deg2Rad(TokenOpt<float>(sc));

                sc.SkipWhitespace();

                if (!sc.IsEOF() && sc.Peek() != '=') {
                    color = fnReadColor(sc);
                }

                sc.SkipWhitespace();

                int32_t order = 0;
                if (!sc.IsEOF() && sc.Peek() != '=') {
                    order = TokenOpt<int32_t>(sc);
                }

                sc.SkipWhitespace();

                auto parent = rootStack.empty() ? -1 : rootStack.top();
                auto sid = Add(diam, rot, 0.0f, color, StickShape::CIRCLE, {x, y}, parent);
                m_stickOrdering.Add(order);

                fnUpdateParentStack(sc, sid);
            } break;
            case 'n':
            case 'N': {
                if (sc.IsEOF()) continue;

                float x = sc.Token<float>();
                float y = sc.Token<float>();
                float len = sc.Token<float>();
                float rot = Deg2Rad(TokenOpt<float>(sc));

                sc.SkipWhitespace();

                auto parent = rootStack.empty() ? -1 : rootStack.top();
                auto sid = Add(len, rot, 0.0f, olc::Colour::WHITE, StickShape::NONE, {x, y}, parent);
                m_stickOrdering.Add(0); // NONE sticks aren't drawn, but m_stickOrdering is indexed by StickID and must stay aligned

                fnUpdateParentStack(sc, sid);
            } break;
            case 'a':
            case 'A': {
                int durationFrames = sc.Token<int>();
                sc.SkipWhitespace();
                if (sc.IsEOF()) continue;

                AnimationMode mode = fnReadAnimationMode(sc);

                sc.SkipWhitespace();

                if (sc.Peek() != '=') continue; // has to name it
                sc.Read();
                sc.SkipWhitespace();

                currentAnimation = sc.Token<std::string>();

                m_animations[currentAnimation] = Animation{
                    0, uint32_t(durationFrames), mode
                };
                m_animtationTracks[currentAnimation] = {};
            } break;
            case 'k':
            case 'K': {
                auto stickName = sc.Token<std::string>();
                int frameNo = sc.Token<int>();
                float x = sc.Token<float>();
                float y = sc.Token<float>();
                float rot = Deg2Rad(sc.Token<float>());

                sc.SkipWhitespace();

                int spriteFrame = 0;
                if (!sc.IsEOF() && (isdigit(sc.Peek()) || sc.Peek() == '_')) {
                    spriteFrame = TokenOpt<int>(sc, 0);
                }

                auto sid = GetStickID(stickName);
                m_animtationTracks[currentAnimation][sid][uint32_t(frameNo)] = Keyframe{
                    olc::vf2d{x, y}, rot, spriteFrame
                };
            } break;
            case 's':
            case 'S': {
                // S x y len rot file_name ox oy tiles_x tiles_y [order] = tag
                float x = sc.Token<float>();
                float y = sc.Token<float>();
                float len = sc.Token<float>();
                float rot = Deg2Rad(TokenOpt<float>(sc));

                sc.SkipWhitespace();
                if (sc.Peek() != '"') continue;

                sc.Read();
                auto fileName = sc.Token<std::string>([](char c) { return c == '"'; });
                sc.Read();

                // origin
                int ox = TokenOpt<int>(sc);
                int oy = TokenOpt<int>(sc);
                // tiling
                int tilesX = TokenOpt<int>(sc, 1);
                int tilesY = TokenOpt<int>(sc, 1);

                sc.SkipWhitespace();

                int32_t order = 0;
                if (!sc.IsEOF() && sc.Peek() != '=') {
                    order = TokenOpt<int32_t>(sc);
                }

                sc.SkipWhitespace();

                auto parent = rootStack.empty() ? -1 : rootStack.top();
                auto sid = Add(len, rot, 0.0f, olc::Colour::WHITE, StickShape::SPRITE, {x, y}, parent);
                m_stickOrdering.Add(order);

                ImageRepository::Get().LoadImage(
                    (baseDir / fileName).string(),
                    {ox, oy}, tilesX, tilesY
                );

                auto& stick = m_sticks[sid];
                auto spr = ImageRepository::Get().GetSprite((baseDir / fileName).string());
                if (spr) {
                    stick.sprite = *spr;
                }

                fnUpdateParentStack(sc, sid);
            } break;
            default: break;
        }
    }

    UpdateSize();
}

void Figure::ParseFile(const std::string &fileName)
{
    if (!std::filesystem::is_regular_file(fileName)) {
        return;
    }

    std::ifstream file(fileName);
    if (!file.is_open()) {
        return;
    }

    auto content = std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
    ParseString(content, std::filesystem::path(fileName).parent_path());
}

StickID Figure::GetStickID(const std::string &name)
{
    auto pos = m_stickNames.find(name);
    if (pos == m_stickNames.end()) return -1;
    return pos->second;
}
