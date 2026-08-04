#include "sdlw/focus.h"
#include "sdlw/window.h"

#include <SDL3/SDL.h>

namespace sdlw {

void FocusManager::add(Focusable* w) { if (w) items_.push_back(w); }
void FocusManager::clear() { items_.clear(); focused_ = -1; }

Focusable* FocusManager::focused() const {
    return (focused_ >= 0 && focused_ < int(items_.size())) ? items_[focused_] : nullptr;
}

void FocusManager::transition(int newIndex, Window& win) {
    if (newIndex == focused_) return;
    if (focused_ >= 0 && focused_ < int(items_.size())) items_[focused_]->setFocus(false, win);
    focused_ = newIndex;
    if (focused_ >= 0 && focused_ < int(items_.size())) items_[focused_]->setFocus(true, win);
}

void FocusManager::setFocus(Focusable* w, Window& win) {
    for (int i = 0; i < int(items_.size()); ++i)
        if (items_[i] == w) { transition(i, win); return; }
    transition(-1, win);
}

// Next acceptsFocus index in direction dir (+1/-1), cyclic; -1 if none.
int FocusManager::step(int from, int dir) const {
    int n = int(items_.size());
    if (n == 0) return -1;
    for (int k = 0; k < n; ++k) {
        from = (from + dir + n) % n;
        if (items_[from]->acceptsFocus()) return from;
    }
    return -1;
}

void FocusManager::focusNext(Window& win) {
    transition(step(focused_ < 0 ? -1 : focused_, +1), win);
}
void FocusManager::focusPrev(Window& win) {
    transition(step(focused_ < 0 ? 0 : focused_, -1), win);
}

void FocusManager::update(Window& win) {
    // Click-to-focus: focus the widget under the cursor, or clear focus if the
    // click missed every registered widget.
    if (win.mousePressed()) {
        float mx = win.mouseX(), my = win.mouseY();
        // The focused widget gets first claim on the click — this lets an open
        // dropdown keep focus when its popup overlaps another control.
        Focusable* cur = focused();
        if (!(cur && cur->acceptsFocus() && cur->hitTest(mx, my))) {
            int hit = -1;
            for (int i = 0; i < int(items_.size()); ++i) {
                if (!items_[i]->acceptsFocus()) continue;
                if (items_[i]->hitTest(mx, my)) { hit = i; break; }
            }
            transition(hit, win);
        }
    }

    // Tab / Shift+Tab traversal.
    if (win.keyPressed(Key::Tab)) {
        if (win.modShift()) focusPrev(win);
        else                focusNext(win);
    }
}

void FocusManager::drawFocusRing(SDL_Renderer* renderer) const {
    Focusable* f = focused();
    if (!f) return;
    float x, y, w, h;
    f->focusRect(x, y, w, h);
    SDL_FRect r{ x - ring_.pad, y - ring_.pad, w + 2 * ring_.pad, h + 2 * ring_.pad };
    SDL_SetRenderDrawColor(renderer, ring_.color[0], ring_.color[1], ring_.color[2], 255);
    SDL_RenderRect(renderer, &r);
}

} // namespace sdlw
