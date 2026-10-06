#include "gui.h"

#include "repository.h"

namespace gui {
    void State::Update()
    {
        mousePos = pge->GetMouse().GetPosition();
        mouseDelta = mousePos - prevMousePos;
        prevMousePos = mousePos;

        auto btn = pge->GetMouse().GetButton(0);
        mouseDown = btn.bHeld;
        mousePressed = btn.bPressed;
        mouseReleased = btn.bReleased;

        rectStack.clear();
        rectStack.push_back({ {0,0}, pge->ScreenSize() });

        prevHot = hot;
        hot = 0;
    }

    static bool PointInRect(const olc::vi2d& p, const olc::vi2d& pos, const olc::vi2d& size) {
        return p.x >= pos.x && p.x < pos.x + size.x &&
            p.y >= pos.y && p.y < pos.y + size.y;
    }

    ID StrID(const std::string &str)
    {
        return static_cast<ID>(std::hash<std::string>{}(str));
    }

    WidgetState ClickableArea(State &gui, ID id)
    {
        if (gui.rectStack.empty()) return WidgetState::None;
        auto [pos, size] = gui.rectStack.back();

        bool inside = PointInRect(gui.mousePos, pos, size);
        WidgetState state = WidgetState::None;

        if (inside && gui.active == 0) {
            gui.hot = id;
            state = WidgetState::Hot;
            if (gui.prevHot != id) SRG("assets/sounds/ui-hover.wav")->Play(false);
            if (gui.mousePressed) {
                gui.active = id;
                state = WidgetState::Active;
            }
        }

        if (gui.active == id && gui.mouseReleased) {
            if (inside) {
                state = WidgetState::Clicked;
                SRG("assets/sounds/ui-confirm.wav")->Play(false);
            }
            gui.active = 0;
        }
        else if (gui.active == id) {
            state = WidgetState::Active;
        }

        return state;
    }

    WidgetState DraggableArea(State &gui, ID id, olc::vi2d &outDelta)
    {
        if (gui.rectStack.empty()) return WidgetState::None;
        auto [pos, size] = gui.rectStack.back();

        outDelta = { 0, 0 };
        bool inside = PointInRect(gui.mousePos, pos, size);
        WidgetState state = WidgetState::None;

        if (inside && gui.active == 0) {
            gui.hot = id;
            state = WidgetState::Hot;
            if (gui.mousePressed) {
                gui.active = id;
                state = WidgetState::Active;
            }
        }

        if (gui.active == id) {
            outDelta = gui.mouseDelta;
            state = WidgetState::Dragging;
            if (gui.mouseReleased) gui.active = 0;
        }

        return state;
    }
    
    bool Option(State &gui, sID id, int &value, const std::string *options, size_t numOptions)
    {
        if (numOptions == 0) return false;

        value = std::clamp(value, 0, static_cast<int>(numOptions - 1));
        return Number(
            gui,
            id,
            value,
            1,
            0,
            static_cast<int>(numOptions - 1),
            options[value],
            true
        );
    }

    // pushes a copy of the current rect
    void BeginContainer(State &gui)
    {
        if (!gui.rectStack.empty()) gui.rectStack.push_back(gui.rectStack.back());
    }

    void EndContainer(State &gui)
    {
        if (!gui.rectStack.empty()) {
            gui.rectStack.pop_back();
        }
    }

    void Label(State &gui, const std::string &text, const olc::vf2d &scale, const olc::Pixel &color)
    {
        if (gui.rectStack.empty()) return;
        auto [pos, size] = gui.rectStack.back();
        gui.pge->GetDraw().StringProp(pos, text, color, scale);
    }

    bool Button(State &gui, sID id, const std::string &text, const olc::vf2d& scale)
    {
        if (gui.rectStack.empty()) return false;
        auto [pos, size] = gui.rectStack.back();
        auto state = ClickableArea(gui, StrID(id));

        auto& draw = gui.pge->GetDraw();

        switch (state) {
            case WidgetState::Hot:
                draw.FilledRect(pos, size, olc::Pixel(255, 255, 255, 80));
                break;
            case WidgetState::Active:
                draw.FilledRect(pos, size, olc::Pixel(0, 0, 0));
                break;
            default:
                draw.FilledRect(pos, size, olc::Pixel(0, 0, 0, 128));
                break;
        }

        auto textSize = draw.GetTextSize(text, true, scale);
        draw.StringProp(pos + (size - textSize) / 2, text, olc::Colour::WHITE, scale);

        draw.Rect(pos, size);

        return state == WidgetState::Clicked;
    }

    Rect ContainerTop(State &gui, int height)
    {
        auto& [pos, size] = gui.rectStack.back();
        Rect cut{pos, {size.x, height}};
        pos.y += height;
        size.y -= height;
        return cut;
    }

    Rect ContainerBottom(State &gui, int height)
    {
        auto& [pos, size] = gui.rectStack.back();
        size.y -= height;
        return {pos + olc::vi2d{0, size.y}, {size.x, height}};
    }

    Rect ContainerLeft(State &gui, int width)
    {
        auto& [pos, size] = gui.rectStack.back();
        Rect cut{pos, {width, size.y}};
        pos.x += width;
        size.x -= width;
        return cut;
    }

    Rect ContainerRight(State &gui, int width)
    {
        auto& [pos, size] = gui.rectStack.back();
        size.x -= width;
        return {pos + olc::vi2d{size.x, 0}, {width, size.y}};
    }

    Rect ContainerRest(State &gui)
    {
        return gui.rectStack.back();
    }

    void ContainerRect(State &gui, Rect rect)
    {
        gui.rectStack.push_back(rect);
    }

    Rect ContainerPop(State &gui)
    {
        if (!gui.rectStack.empty()) {
            Rect rect = gui.rectStack.back();
            gui.rectStack.pop_back();
            return rect;
        }
        return {{0, 0}, {0, 0}};
    }

}
