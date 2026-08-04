// sdlw::ProgressBar - a horizontal value bar (0..1), optionally showing percent.
#pragma once

struct SDL_Renderer;

namespace sdlw {

class Font;

class ProgressBar {
public:
    struct Style {
        unsigned char track[3]  = { 44,  44,  54 };
        unsigned char fill[3]   = { 90,  160, 235 };
        unsigned char border[3] = { 90,  90,  110 };
        unsigned char text[3]   = { 235, 235, 240 };
    };

    ProgressBar() = default;
    ProgressBar(float x, float y, float w, float h);

    void setRect(float x, float y, float w, float h);
    void setValue(float v01);            // clamped to [0, 1]
    void setShowPercent(bool on) { showPercent_ = on; }
    float value() const { return value_; }
    Style& style() { return style_; }

    void draw(SDL_Renderer* renderer, Font& font);

private:
    float x_ = 0, y_ = 0, w_ = 0, h_ = 0;
    float value_ = 0;
    bool  showPercent_ = false;
    Style style_;
};

} // namespace sdlw
