#include "sdlw/radiogroup.h"
#include "sdlw/window.h"
#include "sdlw/font.h"

#include <SDL3/SDL.h>

#include <cmath>
#include <utility>

namespace sdlw {

namespace {
// Filled circle via horizontal scanline spans (SDL has no circle primitive).
void fillCircle(SDL_Renderer* r, float cx, float cy, float rad) {
    int ri = int(rad);
    for (int dy = -ri; dy <= ri; ++dy) {
        float dx = std::sqrt(rad * rad - float(dy) * float(dy));
        SDL_RenderLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}
} // namespace

RadioGroup::RadioGroup(float x, float y, float w, std::vector<std::string> options, float rowHeight)
    : options_(std::move(options)), x_(x), y_(y), w_(w), rowH_(rowHeight) {
    if (!options_.empty()) selected_ = 0;
}

void RadioGroup::setOptions(std::vector<std::string> options) {
    options_ = std::move(options);
    if (selected_ >= count()) selected_ = count() - 1;
}
void RadioGroup::setSelected(int index) {
    selected_ = (index >= 0 && index < count()) ? index : -1;
}
const std::string* RadioGroup::selectedOption() const {
    return (selected_ >= 0 && selected_ < count()) ? &options_[selected_] : nullptr;
}

bool RadioGroup::update(Window& win) {
    float mx = win.mouseX(), my = win.mouseY();
    hover_ = -1;
    if (mx >= x_ && mx < x_ + w_) {
        int row = int((my - y_) / rowH_);
        if (row >= 0 && row < count() && my >= y_) hover_ = row;
    }
    if (win.mousePressed() && hover_ >= 0 && hover_ != selected_) {
        selected_ = hover_;
        return true;
    }
    return false;
}

void RadioGroup::draw(SDL_Renderer* renderer, Font& font) {
    float rad = 8.0f;
    for (int i = 0; i < count(); ++i) {
        float rowY = y_ + i * rowH_;
        float cx = x_ + rad + 1;
        float cy = rowY + rowH_ * 0.5f;

        const unsigned char* ring = (i == hover_) ? style_.ringHover : style_.ring;
        SDL_SetRenderDrawColor(renderer, ring[0], ring[1], ring[2], 255);
        fillCircle(renderer, cx, cy, rad);
        // Hollow it out to the configured background color.
        SDL_SetRenderDrawColor(renderer, style_.bg[0], style_.bg[1], style_.bg[2], 255);
        fillCircle(renderer, cx, cy, rad - 1.6f);
        if (i == selected_) {
            SDL_SetRenderDrawColor(renderer, style_.dot[0], style_.dot[1], style_.dot[2], 255);
            fillCircle(renderer, cx, cy, rad - 4.0f);
        }

        float tx = x_ + rad * 2 + 10;
        float ty = rowY + (rowH_ - font.lineHeight()) * 0.5f;
        font.draw(options_[i], tx, ty, style_.text[0], style_.text[1], style_.text[2]);
    }
}

} // namespace sdlw
