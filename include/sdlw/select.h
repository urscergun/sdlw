// sdlw::Select - a non-editable drop-down (a "select"/"choice" control).
//
// Shows the current selection with a drop-down arrow; clicking opens a ListBox
// popup of all options; clicking an item (or Up/Down + Enter while open) picks
// it. Unlike ComboBox there is no text field or filtering.
//
// Draw a Select AFTER other widgets so its popup renders on top.
#pragma once

#include "sdlw/listbox.h"

#include <string>
#include <vector>

struct SDL_Renderer;

namespace sdlw {

class Window;
class Font;

class Select {
public:
    struct Style {
        unsigned char bg[3]     = { 46,  46,  58 };
        unsigned char border[3] = { 90,  90,  110 };
        unsigned char text[3]   = { 230, 230, 236 };
        unsigned char arrow[3]  = { 200, 200, 210 };
        unsigned char placeholder[3] = { 140, 140, 155 };
    };

    Select() = default;
    Select(float x, float y, float w, float h);

    void setRect(float x, float y, float w, float h);
    void setItems(std::vector<std::string> items);
    void setSelected(int index);
    void setPlaceholder(std::string text);
    void setMaxVisibleRows(int rows);

    int  selected() const { return selected_; }
    const std::string* selectedItem() const;
    bool isOpen() const { return open_; }
    ListBox& list() { return list_; }
    Style&   style() { return style_; }

    // Returns true on the frame a new item is chosen. Call once per frame after
    // Window::pumpEvents().
    bool update(Window& win, Font& font);
    void draw(SDL_Renderer* renderer, Font& font);

private:
    void layout(Font& font);

    ListBox list_;
    Style   style_;
    std::vector<std::string> items_;
    std::string placeholder_;
    float x_ = 0, y_ = 0, w_ = 0, h_ = 0;
    int   selected_ = -1;
    int   maxVisibleRows_ = 8;
    bool  open_ = false;
};

} // namespace sdlw
