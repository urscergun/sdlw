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

// Geometry along the scroll axis (Y for vertical, X for horizontal).
void Scrollbar::thumbGeom(float& thumbStart, float& thumbLen) const {
    float trackStart = (orient_ == Orient::Vertical) ? y_ : x_;
    float trackLen   = (orient_ == Orient::Vertical) ? h_ : w_;
    float frac = (content_ > 0) ? std::min(1.0f, view_ / content_) : 1.0f;
    thumbLen = std::max(20.0f, trackLen * frac);
    float maxS = maxScroll();
    float t = (maxS > 0) ? (value_ / maxS) : 0.0f;
    thumbStart = trackStart + t * (trackLen - thumbLen);
}

bool Scrollbar::update(Window& win) {
    if (!needed()) { dragging_ = false; value_ = 0; return false; }

    bool vert = (orient_ == Orient::Vertical);
    float along = vert ? win.mouseY() : win.mouseX();
    float trackStart = vert ? y_ : x_;
    float trackLen   = vert ? h_ : w_;
    bool down = win.mouseDown();
    float old = value_;

    float thumbStart, thumbLen;
    thumbGeom(thumbStart, thumbLen);

    if (win.mousePressed() && hit(win.mouseX(), win.mouseY())) {
        if (along >= thumbStart && along < thumbStart + thumbLen) {  // grab the thumb
            dragging_ = true;
            grab_ = along - thumbStart;
        } else {                                                      // page toward the click
            value_ += (along < thumbStart) ? -view_ : view_;
        }
    }

    if (down && dragging_) {                            // map thumb position -> value
        float denom = trackLen - thumbLen;
        float t = (denom > 0) ? (along - grab_ - trackStart) / denom : 0.0f;
        value_ = std::clamp(t, 0.0f, 1.0f) * maxScroll();
    }
    if (!down) dragging_ = false;

    value_ = std::clamp(value_, 0.0f, maxScroll());
    return value_ != old;
}

void Scrollbar::draw(SDL_Renderer* renderer) const {
    if (!needed()) return;
    float thumbStart, thumbLen;
    thumbGeom(thumbStart, thumbLen);
    const unsigned char* c = dragging_ ? style_.thumbActive : style_.thumb;
    SDL_SetRenderDrawColor(renderer, c[0], c[1], c[2], 255);
    SDL_FRect thumb = (orient_ == Orient::Vertical)
                          ? SDL_FRect{ x_, thumbStart, w_, thumbLen }
                          : SDL_FRect{ thumbStart, y_, thumbLen, h_ };
    SDL_RenderFillRect(renderer, &thumb);
}

} // namespace sdlw
