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

#include <string>
#include <vector>

struct SDL_Renderer;

namespace sdlw {

class Window;
class Font;

class ComboBox {
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
    std::string lastText_;              // field text at last refilter
};

} // namespace sdlw
