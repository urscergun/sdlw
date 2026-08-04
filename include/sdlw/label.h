// sdlw::Label - static (non-interactive) text with alignment.
#pragma once

#include <string>

struct SDL_Renderer;

namespace sdlw {

class Font;

class Label {
public:
    enum class Align { Left, Center, Right };
    struct Style { unsigned char color[3] = { 225, 225, 232 }; };

    Label() = default;
    // If w <= 0 the text is drawn at (x, y) as a top-left origin. Otherwise it
    // is aligned horizontally within [x, x+w] and vertically centered in h.
    Label(std::string text, float x, float y, float w = 0, float h = 0);

    void setText(std::string text);
    void setAlign(Align a) { align_ = a; }
    void setRect(float x, float y, float w, float h) { x_ = x; y_ = y; w_ = w; h_ = h; }
    Style& style() { return style_; }
    const std::string& text() const { return text_; }

    void draw(SDL_Renderer* renderer, Font& font);

private:
    std::string text_;
    float x_ = 0, y_ = 0, w_ = 0, h_ = 0;
    Align align_ = Align::Left;
    Style style_;
};

} // namespace sdlw
