// sdlw::Button - a minimal clickable UI button.
//
// Draws a filled rectangle with a border and a centered text label (via
// sdlw::Font), and tracks hover/press state. A "click" is a press and release
// that both land inside the button. Uses only SDL.
#pragma once

#include "sdlw/focus.h"

#include <string>

struct SDL_Renderer;

namespace sdlw {

class Font;
class Window;

class Button : public Focusable {
public:
    // Per-state RGB colors. Tweak via style()/setStyle().
    struct Style {
        unsigned char normal[3]  = { 60,  60,  72 };
        unsigned char hover[3]   = { 82,  82, 100 };
        unsigned char pressed[3] = { 44,  44,  54 };
        unsigned char border[3]  = { 110, 110, 132 };
        unsigned char text[3]    = { 235, 235, 240 };
    };

    Button() = default;
    Button(std::string label, float x, float y, float w, float h);

    void setRect(float x, float y, float w, float h);
    void setLabel(std::string label);
    void setStyle(const Style& style);
    Style& style();

    // Update hover/press state from the window's input. Returns true once on
    // the frame the button is clicked (press + release both inside). Call once
    // per frame, after the window has pumped events.
    bool update(Window& win);

    // Draw the button. `font` supplies the label glyphs.
    void draw(SDL_Renderer* renderer, Font& font);

    bool hovered() const { return hovered_; }
    bool pressed() const { return pressed_; }

    // Focusable
    void focusRect(float& x, float& y, float& w, float& h) const override { x = x_; y = y_; w = w_; h = h_; }
    void setFocus(bool f, Window&) override { focused_ = f; }
    bool focused() const override { return focused_; }

private:
    std::string label_;
    float x_ = 0, y_ = 0, w_ = 0, h_ = 0;
    Style style_;
    bool hovered_ = false;
    bool pressed_ = false;
    bool armed_   = false; // press started inside this button
    bool wasDown_ = false; // left button state last frame
    bool focused_ = false;
};

} // namespace sdlw
