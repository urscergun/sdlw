#include "sdlw/checkbox.h"
#include "sdlw/window.h"
#include "sdlw/font.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <utility>

namespace sdlw {

Checkbox::Checkbox(std::string label, float x, float y, float w, float h)
    : label_(std::move(label)), x_(x), y_(y), w_(w), h_(h) {}

void Checkbox::setRect(float x, float y, float w, float h) { x_ = x; y_ = y; w_ = w; h_ = h; }
void Checkbox::setLabel(std::string label) { label_ = std::move(label); }

bool Checkbox::update(Window& win) {
    float mx = win.mouseX(), my = win.mouseY();
    hovered_ = (mx >= x_ && mx < x_ + w_ && my >= y_ && my < y_ + h_);
    if (win.mousePressed() && hovered_) {
        checked_ = !checked_;
        return true;
    }
    return false;
}

void Checkbox::draw(SDL_Renderer* renderer, Font& font) {
    float boxSz = std::min(h_, 20.0f);
    float by = y_ + (h_ - boxSz) * 0.5f;
    SDL_FRect box{ x_, by, boxSz, boxSz };

    const unsigned char* bg = checked_ ? style_.checkBg : (hovered_ ? style_.boxHover : style_.box);
    SDL_SetRenderDrawColor(renderer, bg[0], bg[1], bg[2], 255);
    SDL_RenderFillRect(renderer, &box);
    SDL_SetRenderDrawColor(renderer, style_.border[0], style_.border[1], style_.border[2], 255);
    SDL_RenderRect(renderer, &box);

    if (checked_) {
        // A check mark drawn as two line strokes.
        SDL_SetRenderDrawColor(renderer, style_.check[0], style_.check[1], style_.check[2], 255);
        float l = x_, t = by;
        SDL_RenderLine(renderer, l + boxSz * 0.22f, t + boxSz * 0.52f,
                                 l + boxSz * 0.42f, t + boxSz * 0.72f);
        SDL_RenderLine(renderer, l + boxSz * 0.42f, t + boxSz * 0.72f,
                                 l + boxSz * 0.78f, t + boxSz * 0.28f);
    }

    if (!label_.empty()) {
        float tx = x_ + boxSz + 8;
        float ty = y_ + (h_ - font.lineHeight()) * 0.5f;
        font.draw(label_, tx, ty, style_.text[0], style_.text[1], style_.text[2]);
    }
}

} // namespace sdlw
