#pragma once

#include "olcPixelGameEngine3.h"
#include <cmath>
#include <type_traits>
#include <format>

namespace gui {

    using ID = uint32_t;
    using sID = std::string;

    enum class WidgetState {
        None = 0,
        Hot,
        Active,
        Clicked,
        Dragging
    };

    using Rect = std::tuple<olc::vi2d, olc::vi2d>;

    struct State {
        State() = default;
        State(olc::PixelGameEngine* pge) : pge(pge) {}

        olc::PixelGameEngine* pge;

        olc::vi2d mousePos{};
        olc::vi2d mouseDelta{};

        bool mouseDown{ false }, mousePressed{ false }, mouseReleased{ false };

        ID hot{ 0 }, active{ 0 };
        ID prevHot{ 0 }; // hot id of the previous frame, used to detect hover start

        std::vector<Rect> rectStack{};

        void Update();

        bool IsHot(ID id) const { return hot == id; }
        bool IsActive(ID id) const { return active == id; }
        bool IsMouseBusy() const { return active != 0; }

    private:
        olc::vi2d prevMousePos{};
    };

    ID StrID(const std::string& str);
    WidgetState ClickableArea(State& gui, ID id);
    WidgetState DraggableArea(State& gui, ID id, olc::vi2d& outDelta);

    void Label(State& gui, const std::string& text, const olc::vf2d& scale = {1,1}, const olc::Pixel& color = olc::Colour::WHITE);
    bool Button(State& gui, sID id, const std::string& text, const olc::vf2d& scale = {1,1});
    void DrawButton(State& gui, Rect r, WidgetState state, const std::string& text, const olc::vf2d& scale = {1,1});

    void ContainerRect(State& gui, Rect rect);
    Rect ContainerPop(State& gui);

    void BeginContainer(State& gui); // pushes a copy of the current rect
    // rect-cut operations
    Rect ContainerTop(State& gui, int height);
    Rect ContainerBottom(State& gui, int height);
    Rect ContainerLeft(State& gui, int width);
    Rect ContainerRight(State& gui, int width);
    //
    void EndContainer(State& gui);

    bool Option(State &gui, sID id, int &value, const std::string *options, size_t numOptions);

    template <typename T>
        requires std::is_arithmetic_v<T>
    bool Number(State& gui, sID id, T& value, T step = 1, T lo = 0, T hi = 0, const std::string& fmt = "{}", bool cyclical = false) {
        if (gui.rectStack.empty()) return false;
        bool changed = false;
        auto [pos, size] = gui.rectStack.back();

        olc::vi2d delta;
        ContainerRect(gui, {pos + olc::vi2d{8, 0}, {size.x - 16, size.y}});
        auto drag = DraggableArea(gui, StrID(id + "#d"), delta);
        ContainerPop(gui);

        ContainerRect(gui, {pos, {8, size.y}});
        auto dec = ClickableArea(gui, StrID(id + "#-"));
        ContainerPop(gui);

        ContainerRect(gui, {pos + olc::vi2d{size.x - 8, 0}, {8, size.y}});
        auto inc = ClickableArea(gui, StrID(id + "#+"));
        ContainerPop(gui);

        if (drag == WidgetState::Dragging) {
            value += step * delta.x;
            changed = true;
        }

        if (dec == WidgetState::Clicked) { value -= step; changed = true; }
        else if (inc == WidgetState::Clicked) { value += step; changed = true; }

        if (lo != hi && lo < hi) {
            if (cyclical) {
                if constexpr (std::is_integral_v<T>) {
                    auto span = hi - lo + 1;
                    value = lo + ((value - lo) % span + span) % span;
                }
                else {
                    auto span = hi - lo;
                    value = lo + std::fmod(std::fmod(value - lo, span) + span, span);
                }
            }
            else {
                value = std::clamp(value, lo, hi);
            }
        }

        auto& draw = gui.pge->GetDraw();

        draw.FilledRect(
            pos,
            {8, float(size.y)},
            dec == WidgetState::Active ? olc::Colour::WHITE : olc::Colour::YELLOW
        );

        draw.FilledRect(
            pos + olc::vi2d{size.x-8,0},
            {8, float(size.y)},
            inc == WidgetState::Active ? olc::Colour::WHITE : olc::Colour::YELLOW
        );

        draw.String(pos + olc::vi2d{2, 2}, "<", olc::Colour::BLACK);
        draw.String(pos + olc::vi2d{size.x-6,2}, ">", olc::Colour::BLACK);

        auto text = std::vformat(fmt, std::make_format_args(value));
        auto textSize = draw.GetTextSize(text, true);

        int xCenter = size.x / 2 - textSize.x / 2;
        draw.StringProp(pos + olc::vi2d{xCenter, 2}, text, olc::Colour::YELLOW);

        draw.Rect(pos, size, olc::Colour::YELLOW);

        return changed;
    }

    // Rect overloads: draw the widget inside `r` (pushes and pops it for you).
    // e.g. gui::Button(ui, gui::ContainerTop(ui, 24), "resume", "Resume")
    struct RectScope {
        State& gui;
        RectScope(State& gui, Rect r) : gui(gui) { ContainerRect(gui, r); }
        ~RectScope() { ContainerPop(gui); }
    };

    inline void Label(State& gui, Rect r, const std::string& text, const olc::vf2d& scale = {1,1}, const olc::Pixel& color = olc::Colour::WHITE) {
        RectScope s{gui, r};
        Label(gui, text, scale, color);
    }

    inline bool Button(State& gui, Rect r, sID id, const std::string& text, const olc::vf2d& scale = {1,1}) {
        RectScope s{gui, r};
        return Button(gui, id, text, scale);
    }

    inline bool Option(State& gui, Rect r, sID id, int& value, const std::string* options, size_t numOptions) {
        RectScope s{gui, r};
        return Option(gui, id, value, options, numOptions);
    }

    template <typename T>
        requires std::is_arithmetic_v<T>
    bool Number(State& gui, Rect r, sID id, T& value, T step = 1, T lo = 0, T hi = 0, const std::string& fmt = "{}", bool cyclical = false) {
        RectScope s{gui, r};
        return Number(gui, id, value, step, lo, hi, fmt, cyclical);
    }
}
