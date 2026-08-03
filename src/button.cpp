#include "sdlw/button.h"
#include "sdlw/font.h"

#include <SDL3/SDL.h>

#include <utility>

namespace sdlw {

Button::Button(std::string label, float x, float y, float w, float h)
    : label_(std::move(label)), x_(x), y_(y), w_(w), h_(h) {}

void Button::setRect(float x, float y, float w, float h) { x_ = x; y_ = y; w_ = w; h_ = h; }
void Button::setLabel(std::string label) { label_ = std::move(label); }
void Button::setStyle(const Style& style) { style_ = style; }
Button::Style& Button::style() { return style_; }

bool Button::update() {
    float mx = 0, my = 0;
    SDL_MouseButtonFlags buttons = SDL_GetMouseState(&mx, &my);
    bool down = (buttons & SDL_BUTTON_LMASK) != 0;

    hovered_ = (mx >= x_ && mx < x_ + w_ && my >= y_ && my < y_ + h_);

    bool clicked = false;
    if (down && !wasDown_) {          // press edge
        if (hovered_) armed_ = true;
    } else if (!down && wasDown_) {    // release edge
        if (armed_ && hovered_) clicked = true;
        armed_ = false;
    }
    pressed_ = down && armed_ && hovered_;
    wasDown_ = down;
    return clicked;
}

void Button::draw(SDL_Renderer* renderer, Font& font) {
    SDL_FRect rect{ x_, y_, w_, h_ };

    const unsigned char* bg = pressed_ ? style_.pressed
                            : hovered_ ? style_.hover
                                       : style_.normal;
    SDL_SetRenderDrawColor(renderer, bg[0], bg[1], bg[2], 255);
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, style_.border[0], style_.border[1], style_.border[2], 255);
    SDL_RenderRect(renderer, &rect);

    // Center the label within the button.
    int tw = 0, th = 0;
    font.measure(label_, &tw, &th);
    float tx = x_ + (w_ - tw) * 0.5f;
    float ty = y_ + (h_ - th) * 0.5f;
    // Nudge down by 1px when pressed for a subtle push effect.
    if (pressed_) ty += 1.0f;
    font.draw(label_, tx, ty, style_.text[0], style_.text[1], style_.text[2]);
}

} // namespace sdlw
