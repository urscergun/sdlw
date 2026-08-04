#include "sdlw/label.h"
#include "sdlw/font.h"

#include <utility>

namespace sdlw {

Label::Label(std::string text, float x, float y, float w, float h)
    : text_(std::move(text)), x_(x), y_(y), w_(w), h_(h) {}

void Label::setText(std::string text) { text_ = std::move(text); }

void Label::draw(SDL_Renderer* renderer, Font& font) {
    (void)renderer;
    if (w_ <= 0) {
        font.draw(text_, x_, y_, style_.color[0], style_.color[1], style_.color[2]);
        return;
    }
    int tw = 0, th = 0;
    font.measure(text_, &tw, &th);
    float tx = x_;
    if (align_ == Align::Center)     tx = x_ + (w_ - tw) * 0.5f;
    else if (align_ == Align::Right) tx = x_ + (w_ - tw);
    float ty = (h_ > 0) ? y_ + (h_ - font.lineHeight()) * 0.5f : y_;
    font.draw(text_, tx, ty, style_.color[0], style_.color[1], style_.color[2]);
}

} // namespace sdlw
