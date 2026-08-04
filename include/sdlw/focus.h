// sdlw::Focusable + sdlw::FocusManager - keyboard focus and Tab traversal.
//
// Widgets that can hold keyboard focus implement Focusable. A FocusManager owns
// an ordered list of them and moves focus with Tab / Shift+Tab, syncs focus to
// mouse clicks, and can draw a focus ring around the focused widget.
//
// Usage:
//     FocusManager focus;
//     focus.add(&name); focus.add(&agree); focus.add(&okButton);
//     // per frame:
//     win.pumpEvents();
//     focus.update(win);          // BEFORE the widgets' own update()
//     name.update(win, ui);
//     agree.update(win);
//     okButton.update(win);
//     ... draw widgets ...
//     focus.drawFocusRing(win.renderer());
#pragma once

#include <vector>

struct SDL_Renderer;

namespace sdlw {

class Window;

class Focusable {
public:
    virtual ~Focusable() = default;

    // Bounding rect used for the focus ring and click-to-focus hit testing.
    virtual void focusRect(float& x, float& y, float& w, float& h) const = 0;

    // The manager sets focus state here. `win` is provided for side effects
    // such as starting/stopping OS text input; widgets that don't need it
    // ignore it.
    virtual void setFocus(bool focused, Window& win) = 0;

    virtual bool focused() const = 0;
    virtual bool acceptsFocus() const { return true; }
};

class FocusManager {
public:
    struct RingStyle {
        unsigned char color[3] = { 120, 170, 235 };
        float pad = 2.0f;
    };

    void add(Focusable* w);                 // tab order = insertion order
    void clear();

    // Handle Tab/Shift+Tab traversal and click-to-focus. Call once per frame
    // after pumpEvents() and before the widgets' own update() calls.
    void update(Window& win);

    Focusable* focused() const;
    void setFocus(Focusable* w, Window& win);
    void focusNext(Window& win);
    void focusPrev(Window& win);

    // Draw a ring around the focused widget (call after drawing the widgets).
    void drawFocusRing(SDL_Renderer* renderer) const;
    RingStyle& ringStyle() { return ring_; }

private:
    void transition(int newIndex, Window& win);
    int  step(int from, int dir) const;     // next/prev acceptsFocus index (cyclic)

    std::vector<Focusable*> items_;
    int      focused_ = -1;
    RingStyle ring_;
};

} // namespace sdlw
