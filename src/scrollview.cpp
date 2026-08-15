#include "sdlw/scrollview.h"
#include "sdlw/window.h"

#include <SDL3/SDL.h>

namespace sdlw {

namespace {
constexpr float kPad = 6.0f;   // content inset
constexpr float kBarW = 7.0f;  // scrollbar width
}

ScrollView::ScrollView(float x, float y, float w, float h) { setRect(x, y, w, h); }

void ScrollView::setRect(float x, float y, float w, float h) { x_ = x; y_ = y; w_ = w; h_ = h; }

// Size the scrollbar from the current rect + content height. Called from both
// update() and beginContent() so the bar tracks the panel even during a live
// resize (when only the draw path runs, not update()).
void ScrollView::layoutBar() {
    float innerH = h_ - 2;
    bar_.setRect(x_ + w_ - kBarW - 1, y_ + 1, kBarW, innerH);
    bar_.setRange(contentH_, innerH);
}

float ScrollView::viewHeight() const { return h_ - 2; }
float ScrollView::viewWidth() const {
    return w_ - 2 * kPad - (bar_.needed() ? kBarW + 2 : 0);
}
float ScrollView::contentX() const { return x_ + kPad; }
float ScrollView::contentTop() const { return y_ + kPad - bar_.value(); }

bool ScrollView::update(Window& win) {
    layoutBar();
    float innerH = h_ - 2;

    float old = bar_.value();
    float mx = win.mouseX(), my = win.mouseY();
    bool overView = (mx >= x_ && mx < x_ + w_ && my >= y_ && my < y_ + h_);

    if (win.mousePressed()) focused_ = overView;      // click to focus (blur if outside)

    if (overView && win.mouseWheel() != 0) {
        bar_.setValue(bar_.value() - win.mouseWheel() * 40.0f); // ~40px per notch
    }

    // Keyboard scrolling when focused.
    if (focused_) {
        if (win.keyPressed(Key::Up))       bar_.setValue(bar_.value() - 40.0f);
        if (win.keyPressed(Key::Down))     bar_.setValue(bar_.value() + 40.0f);
        if (win.keyPressed(Key::PageUp))   bar_.setValue(bar_.value() - innerH);
        if (win.keyPressed(Key::PageDown)) bar_.setValue(bar_.value() + innerH);
        if (win.keyPressed(Key::Home))     bar_.setValue(0);
        if (win.keyPressed(Key::End))      bar_.setValue(bar_.maxScroll());
    }

    bar_.update(win);
    return bar_.value() != old;
}

void ScrollView::beginContent(SDL_Renderer* renderer) {
    layoutBar();   // keep the bar sized to the panel even during live resize
    SDL_FRect bg{ x_, y_, w_, h_ };
    SDL_SetRenderDrawColor(renderer, style_.bg[0], style_.bg[1], style_.bg[2], 255);
    SDL_RenderFillRect(renderer, &bg);

    SDL_Rect clip{ int(x_) + 1, int(y_) + 1, int(w_) - 2, int(h_) - 2 };
    SDL_SetRenderClipRect(renderer, &clip);
}

void ScrollView::endContent(SDL_Renderer* renderer) {
    SDL_SetRenderClipRect(renderer, nullptr);
    bar_.draw(renderer);
    SDL_FRect bg{ x_, y_, w_, h_ };
    SDL_SetRenderDrawColor(renderer, style_.border[0], style_.border[1], style_.border[2], 255);
    SDL_RenderRect(renderer, &bg);
}

} // namespace sdlw
