// sdlw::ComboBox - an editable drop-down (combo) box.
//
// Combines a TextBox field with a drop-down arrow and a ListBox popup. Typing
// filters the list; clicking the arrow (or pressing Down) opens it; clicking an
// item or pressing Enter commits it into the field. Built entirely from the
// existing sdlw::TextBox and sdlw::ListBox widgets.
//
// Because the popup is drawn on top of whatever is beneath it, draw a ComboBox
// AFTER other widgets in the frame.
#pragma once

#include "sdlw/textbox.h"
#include "sdlw/listbox.h"
#include "sdlw/focus.h"

#include <string>
#include <vector>

struct SDL_Renderer;

namespace sdlw {

class Window;
class Font;

class ComboBox : public Focusable {
public:
    struct Style {
        unsigned char arrowBg[3] = { 54,  54,  66 };
        unsigned char arrow[3]   = { 210, 210, 220 };
    };

    ComboBox() = default;
    ComboBox(float x, float y, float w, float h);

    void setRect(float x, float y, float w, float h);
    void setItems(std::vector<std::string> items);
    void setText(std::string text);
    void setMaxVisibleRows(int rows);

    const std::string& text() const;
    bool isOpen() const { return open_; }

    // Access the sub-widgets to style them.
    TextBox& field() { return field_; }
    ListBox& list()  { return list_; }
    Style&   style() { return style_; }

    // Handle the field, arrow, popup, filtering, and keyboard. Returns true on
    // the frame an item is committed (chosen). Call once per frame after
    // Window::pumpEvents().
    bool update(Window& win, Font& font);

    // Draw the field, arrow, and (if open) the popup list on top.
    void draw(SDL_Renderer* renderer, Font& font);

    // Focusable — focus follows the text field; hit test also covers the popup.
    void focusRect(float& x, float& y, float& w, float& h) const override { x = x_; y = y_; w = w_; h = h_; }
    void setFocus(bool f, Window& win) override { field_.setFocus(f, win); if (!f) open_ = false; }
    bool focused() const override { return field_.focused(); }
    bool hitTest(float px, float py) const override {
        if (Focusable::hitTest(px, py)) return true;
        if (open_) { float x, y, w, h; list_.focusRect(x, y, w, h);
                     return px >= x && px < x + w && py >= y && py < y + h; }
        return false;
    }

private:
    void layout(Font& font);            // position field/arrow/list rects
    void refilter();                    // rebuild the list from items_ + text
    void commit(const std::string& item, Window& win);

    TextBox field_;
    ListBox list_;
    Style   style_;
    std::vector<std::string> items_;    // full, unfiltered item list
    float x_ = 0, y_ = 0, w_ = 0, h_ = 0;
    float arrowW_ = 0;                  // width of the arrow button (== h_)
    int   maxVisibleRows_ = 6;
    bool  open_ = false;
    bool  committed_ = true;            // field holds a chosen value (browse all on open)
    std::string lastText_;              // field text at last refilter
};

} // namespace sdlw
