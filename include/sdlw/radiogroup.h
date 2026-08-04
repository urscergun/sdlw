// sdlw::RadioGroup - a vertical set of mutually-exclusive options.
//
// Options are laid out as rows of `rowHeight` starting at (x, y), each `w` wide
// and clickable. update() returns true on the frame the selection changes.
#pragma once

#include "sdlw/focus.h"

#include <string>
#include <vector>

struct SDL_Renderer;

namespace sdlw {

class Window;
class Font;

class RadioGroup : public Focusable {
public:
    struct Style {
        unsigned char ring[3]     = { 130, 130, 150 };
        unsigned char ringHover[3]= { 160, 160, 185 };
        unsigned char dot[3]      = { 90,  160, 235 };
        unsigned char text[3]     = { 225, 225, 232 };
        unsigned char bg[3]       = { 24,  24,  32 }; // color behind the ring (match window)
    };

    RadioGroup() = default;
    RadioGroup(float x, float y, float w, std::vector<std::string> options, float rowHeight = 26);

    void setRect(float x, float y, float w) { x_ = x; y_ = y; w_ = w; }
    void setOptions(std::vector<std::string> options);
    void setRowHeight(float h) { rowH_ = h; }
    void setSelected(int index);
    int  selected() const { return selected_; }
    const std::string* selectedOption() const;
    int  count() const { return int(options_.size()); }
    Style& style() { return style_; }

    bool update(Window& win);                 // true on the frame selection changes
    void draw(SDL_Renderer* renderer, Font& font);

    // Focusable — the ring/hit rect spans all rows.
    void focusRect(float& x, float& y, float& w, float& h) const override {
        x = x_; y = y_; w = w_; h = rowH_ * count();
    }
    void setFocus(bool f, Window&) override { focused_ = f; }
    bool focused() const override { return focused_; }

private:
    std::vector<std::string> options_;
    float x_ = 0, y_ = 0, w_ = 0, rowH_ = 26;
    int   selected_ = -1;
    int   hover_ = -1;
    bool  focused_ = false;
    Style style_;
};

} // namespace sdlw
