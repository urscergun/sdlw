// sdlw::ListBox - a simple scrollable list of text items with selection.
//
// Click an item to select it; hover highlights; the mouse wheel and Up/Down/
// Home/End scroll. A scrollbar thumb appears when the content overflows.
// Draws via sdlw::Font and uses only SDL. Feed input each frame via
// update(Window&, Font&).
#pragma once

#include "sdlw/focus.h"
#include "sdlw/scrollbar.h"

#include <string>
#include <vector>

struct SDL_Renderer;

namespace sdlw {

class Window;
class Font;

class ListBox : public Focusable {
public:
    struct Style {
        unsigned char bg[3]           = { 34,  34,  42 };
        unsigned char border[3]       = { 90,  90, 110 };
        unsigned char item[3]         = { 220, 220, 228 };
        unsigned char hoverBg[3]      = { 52,  52,  64 };
        unsigned char selectedBg[3]   = { 52,  90, 150 };
        unsigned char selectedText[3] = { 245, 245, 250 };
        unsigned char scrollThumb[3]  = { 100, 100, 120 };
    };

    ListBox() = default;
    ListBox(float x, float y, float w, float h);

    void setRect(float x, float y, float w, float h);
    void setItems(std::vector<std::string> items);
    void addItem(std::string item);
    void clear();
    Style& style();

    int count() const { return int(items_.size()); }
    int selected() const { return selected_; }
    void setSelected(int index);
    // Select an item and scroll it into view (for programmatic/keyboard nav).
    void select(int index, Font& font);
    const std::string* selectedItem() const;

    int rowHeight(Font& font) const;        // pixel height of one row

    // Handle hover, selection (mouse + Up/Down/Home/End), and scrolling (wheel
    // + keys). Returns true if the selection changed this frame. Call once per
    // frame after Window::pumpEvents().
    bool update(Window& win, Font& font);

    // True on the frame an item was clicked (even if it was already selected).
    bool itemClicked() const { return itemClicked_; }

    // Draw the list, selection/hover highlights, and scrollbar thumb.
    void draw(SDL_Renderer* renderer, Font& font);

    // Focusable
    void focusRect(float& x, float& y, float& w, float& h) const override { x = x_; y = y_; w = w_; h = h_; }
    void setFocus(bool f, Window&) override { focused_ = f; }
    bool focused() const override { return focused_; }

private:
    float maxScroll(Font& font) const;      // clamp bound for scroll_
    void  scrollToSelected(Font& font);     // keep selected row visible
    void  syncBar(Font& font);              // feed the scrollbar geometry/range

    std::vector<std::string> items_;
    float x_ = 0, y_ = 0, w_ = 0, h_ = 0;
    Style style_;
    int   selected_ = -1;
    int   hover_ = -1;
    float scroll_ = 0;         // vertical scroll offset in pixels
    bool  focused_ = false;
    int   rowPad_ = 6;         // extra vertical padding per row
    bool  itemClicked_ = false;// an item was clicked this frame
    Scrollbar bar_;            // vertical scrollbar (right edge)
};

} // namespace sdlw
