// sdlw::Scrollbar - a reusable vertical scrollbar.
//
// Given a content size and a visible (view) size, it exposes a scroll value in
// [0, maxScroll] driven by dragging the thumb and paging the track. It reads
// input from the Window and draws a thumb within its track rect. Hosts (ListBox,
// ScrollView) own one, feed it geometry + range each frame, and read value().
#pragma once

struct SDL_Renderer;

namespace sdlw {

class Window;

class Scrollbar {
public:
    enum class Orient { Vertical, Horizontal };

    struct Style {
        unsigned char thumb[3]       = { 100, 100, 120 };
        unsigned char thumbActive[3] = { 140, 140, 165 };
    };

    Scrollbar() = default;

    void setOrientation(Orient o) { orient_ = o; }
    void setRect(float x, float y, float w, float h);     // the track rectangle
    void setRange(float contentSize, float viewSize);     // total vs visible
    void setValue(float v);                               // scroll offset (clamped)

    float value() const { return value_; }
    float maxScroll() const;                              // max(0, content - view)
    bool  needed() const { return content_ > view_ + 0.5f; }
    bool  dragging() const { return dragging_; }
    Style& style() { return style_; }

    // Handle thumb drag + track paging. Returns true if the value changed.
    bool update(Window& win);

    // True if the point is within the track rectangle (for host hover suppression).
    bool hit(float px, float py) const;

    void draw(SDL_Renderer* renderer) const;

private:
    void thumbGeom(float& thumbY, float& thumbH) const;

    float x_ = 0, y_ = 0, w_ = 0, h_ = 0;
    float content_ = 0, view_ = 0, value_ = 0;
    bool  dragging_ = false;
    float grab_ = 0;                                      // mouse-to-thumb-start offset
    Orient orient_ = Orient::Vertical;
    Style style_;
};

} // namespace sdlw
