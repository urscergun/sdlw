// sdlw::TextBox - a single-line editable text field.
//
// Click to focus; type to insert; Backspace/Delete/Left/Right/Home/End edit and
// move the caret. Draws a box, the text (via sdlw::Font), a blinking caret, and
// placeholder text when empty. Horizontally scrolls to keep the caret visible.
// Uses only SDL. Feed it input each frame via update(Window&).
#pragma once

#include "sdlw/focus.h"

#include <string>

struct SDL_Renderer;

namespace sdlw {

class Window;
class Font;

class TextBox : public Focusable {
public:
    struct Style {
        unsigned char bg[3]            = { 38,  38,  46 };
        unsigned char bgFocused[3]     = { 46,  46,  58 };
        unsigned char border[3]        = { 90,  90, 110 };
        unsigned char borderFocused[3] = { 120, 170, 235 };
        unsigned char text[3]          = { 235, 235, 240 };
        unsigned char placeholder[3]   = { 130, 130, 145 };
        unsigned char caret[3]         = { 235, 235, 240 };
        unsigned char selection[3]     = { 52,  90, 150 }; // highlight behind selected text
    };

    TextBox() = default;
    TextBox(float x, float y, float w, float h);

    void setRect(float x, float y, float w, float h);
    void setText(std::string text);
    void setPlaceholder(std::string text);
    void setFocused(bool focused, Window& win);
    Style& style();

    const std::string& text() const { return text_; }
    bool focused() const override { return focused_; }
    bool hasSelection() const { return sel_ != caret_; }
    std::string selectedText() const;

    // Handle focus + selection (via mouse), typed text, editing keys, and the
    // Ctrl+A/C/X/V shortcuts for this frame. `font` maps mouse x to a caret
    // position. Call once per frame after Window::pumpEvents().
    void update(Window& win, Font& font);

    // Draw the field, selection highlight, text, and caret.
    void draw(SDL_Renderer* renderer, Font& font);

    // Focusable
    void focusRect(float& x, float& y, float& w, float& h) const override { x = x_; y = y_; w = w_; h = h_; }
    void setFocus(bool f, Window& win) override { setFocused(f, win); }

private:
    std::string text_;
    std::string placeholder_;
    float x_ = 0, y_ = 0, w_ = 0, h_ = 0;
    Style  style_;
    std::size_t caret_ = 0;   // caret position (moving end), in bytes into text_
    std::size_t sel_ = 0;     // selection anchor, in bytes; sel_==caret_ => none
    float  scroll_ = 0;       // horizontal scroll offset in pixels
    bool   focused_ = false;
    bool   dragging_ = false; // selecting with the mouse
};

} // namespace sdlw
