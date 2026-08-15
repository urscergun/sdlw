// sdlw::ScrollView - a scrollable viewport over taller content.
//
// You tell it the total content height; it clips to its rectangle, scrolls
// vertically (mouse wheel + scrollbar), and hands you an origin at which to draw
// your content (already offset by the scroll amount). Foundation for scroll
// panels and a multi-line text area.
//
// Usage:
//     view.setContentHeight(totalHeight);
//     view.update(win);
//     view.beginContent(win.renderer());
//         float ox = view.contentX(), oy = view.contentTop();
//         // draw each item at (ox, oy + itemY) — off-screen parts are clipped
//     view.endContent(win.renderer());
#pragma once

#include "sdlw/scrollbar.h"
#include "sdlw/focus.h"

struct SDL_Renderer;

namespace sdlw {

class Window;

class ScrollView : public Focusable {
public:
    struct Style {
        unsigned char bg[3]     = { 30, 30, 38 };
        unsigned char border[3] = { 90, 90, 110 };
    };

    ScrollView() = default;
    ScrollView(float x, float y, float w, float h);

    void setRect(float x, float y, float w, float h);
    void setContentHeight(float h) { contentH_ = h; }

    float scroll() const { return bar_.value(); }
    float contentX() const;       // left x for drawing content (padded)
    float contentTop() const;     // top y for content (already offset by -scroll)
    float viewWidth() const;      // usable content width (minus scrollbar)
    float viewHeight() const;     // viewport height
    Scrollbar& scrollbar() { return bar_; }
    Style& style() { return style_; }

    // Wheel-over-view + scrollbar drag/paging. Returns true if scroll changed.
    bool update(Window& win);

    // Bracket the content drawing: beginContent fills the background and clips to
    // the viewport; endContent restores the clip and draws the border + scrollbar.
    void beginContent(SDL_Renderer* renderer);
    void endContent(SDL_Renderer* renderer);

    // Focusable — when focused, arrows / PageUp-Down / Home-End scroll it.
    void focusRect(float& x, float& y, float& w, float& h) const override { x = x_; y = y_; w = w_; h = h_; }
    void setFocus(bool f, Window&) override { focused_ = f; }
    bool focused() const override { return focused_; }

private:
    void layoutBar();   // size the scrollbar from the current rect

    Scrollbar bar_;
    float x_ = 0, y_ = 0, w_ = 0, h_ = 0;
    float contentH_ = 0;
    bool  focused_ = false;
    Style style_;
};

} // namespace sdlw
