// sdlw::Checkbox - a labeled on/off toggle.
//
// The whole row (box + label) within [x, x+w] is clickable. update() reads
// input from the Window; returns true on the frame the state toggles.
#pragma once

#include <string>

struct SDL_Renderer;

namespace sdlw {

class Window;
class Font;

class Checkbox {
public:
    struct Style {
        unsigned char box[3]      = { 44,  44,  54 };
        unsigned char boxHover[3] = { 56,  56,  70 };
        unsigned char border[3]   = { 110, 110, 132 };
        unsigned char checkBg[3]  = { 90,  160, 235 };
        unsigned char check[3]    = { 245, 245, 250 };
        unsigned char text[3]     = { 225, 225, 232 };
    };

    Checkbox() = default;
    Checkbox(std::string label, float x, float y, float w, float h);

    void setRect(float x, float y, float w, float h);
    void setLabel(std::string label);
    void setChecked(bool on) { checked_ = on; }
    bool checked() const { return checked_; }
    Style& style() { return style_; }

    bool update(Window& win);                 // true on the frame it toggles
    void draw(SDL_Renderer* renderer, Font& font);

private:
    std::string label_;
    float x_ = 0, y_ = 0, w_ = 0, h_ = 0;
    bool  checked_ = false;
    bool  hovered_ = false;
    Style style_;
};

} // namespace sdlw
