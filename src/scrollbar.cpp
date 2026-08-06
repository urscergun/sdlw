#include "sdlw/scrollbar.h"
#include "sdlw/window.h"

#include <SDL3/SDL.h>

#include <algorithm>

namespace sdlw {

void Scrollbar::setRect(float x, float y, float w, float h) { x_ = x; y_ = y; w_ = w; h_ = h; }
void Scrollbar::setRange(float contentSize, float viewSize) { content_ = contentSize; view_ = viewSize; }
void Scrollbar::setValue(float v) { value_ = std::clamp(v, 0.0f, maxScroll()); }

float Scrollbar::maxScroll() const { return std::max(0.0f, content_ - view_); }

bool Scrollbar::hit(float px, float py) const {
    return px >= x_ && px < x_ + w_ && py >= y_ && py < y_ + h_;
}

void Scrollbar::thumbGeom(float& thumbY, float& thumbH) const {
    float frac = (content_ > 0) ? std::min(1.0f, view_ / content_) : 1.0f;
    thumbH = std::max(20.0f, h_ * frac);
    float maxS = maxScroll();
    float t = (maxS > 0) ? (value_ / maxS) : 0.0f;
    thumbY = y_ + t * (h_ - thumbH);
}

bool Scrollbar::update(Window& win) {
    if (!needed()) { dragging_ = false; value_ = 0; return false; }

    float mx = win.mouseX(), my = win.mouseY();
    bool down = win.mouseDown();
    float old = value_;

    float thumbY, thumbH;
    thumbGeom(thumbY, thumbH);

    if (win.mousePressed() && hit(mx, my)) {
        if (my >= thumbY && my < thumbY + thumbH) {   // grab the thumb
            dragging_ = true;
            grab_ = my - thumbY;
        } else {                                       // page toward the click
            value_ += (my < thumbY) ? -view_ : view_;
        }
    }

    if (down && dragging_) {                            // map thumb position -> value
        float denom = h_ - thumbH;
        float t = (denom > 0) ? (my - grab_ - y_) / denom : 0.0f;
        value_ = std::clamp(t, 0.0f, 1.0f) * maxScroll();
    }
    if (!down) dragging_ = false;

    value_ = std::clamp(value_, 0.0f, maxScroll());
    return value_ != old;
}

void Scrollbar::draw(SDL_Renderer* renderer) const {
    if (!needed()) return;
    float thumbY, thumbH;
    thumbGeom(thumbY, thumbH);
    const unsigned char* c = dragging_ ? style_.thumbActive : style_.thumb;
    SDL_SetRenderDrawColor(renderer, c[0], c[1], c[2], 255);
    SDL_FRect thumb{ x_, thumbY, w_, thumbH };
    SDL_RenderFillRect(renderer, &thumb);
}

} // namespace sdlw
