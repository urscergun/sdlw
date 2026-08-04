#include "sdlw/progressbar.h"
#include "sdlw/font.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstdio>

namespace sdlw {

ProgressBar::ProgressBar(float x, float y, float w, float h) : x_(x), y_(y), w_(w), h_(h) {}

void ProgressBar::setRect(float x, float y, float w, float h) { x_ = x; y_ = y; w_ = w; h_ = h; }
void ProgressBar::setValue(float v01) { value_ = std::clamp(v01, 0.0f, 1.0f); }

void ProgressBar::draw(SDL_Renderer* renderer, Font& font) {
    SDL_FRect track{ x_, y_, w_, h_ };
    SDL_SetRenderDrawColor(renderer, style_.track[0], style_.track[1], style_.track[2], 255);
    SDL_RenderFillRect(renderer, &track);

    if (value_ > 0) {
        SDL_FRect fill{ x_, y_, w_ * value_, h_ };
        SDL_SetRenderDrawColor(renderer, style_.fill[0], style_.fill[1], style_.fill[2], 255);
        SDL_RenderFillRect(renderer, &fill);
    }

    SDL_SetRenderDrawColor(renderer, style_.border[0], style_.border[1], style_.border[2], 255);
    SDL_RenderRect(renderer, &track);

    if (showPercent_) {
        char buf[8];
        std::snprintf(buf, sizeof buf, "%d%%", int(value_ * 100.0f + 0.5f));
        int tw = 0, th = 0;
        font.measure(buf, &tw, &th);
        float tx = x_ + (w_ - tw) * 0.5f;
        float ty = y_ + (h_ - font.lineHeight()) * 0.5f;
        font.draw(buf, tx, ty, style_.text[0], style_.text[1], style_.text[2]);
    }
}

} // namespace sdlw
