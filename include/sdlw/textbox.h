// sdlw::TextBox - a single-line editable text field.
//
// Click to focus; type to insert; Backspace/Delete/Left/Right/Home/End edit and
// move the caret. Draws a box, the text (via sdlw::Font), a blinking caret, and
// placeholder text when empty. Horizontally scrolls to keep the caret visible.
// Uses only SDL. Feed it input each frame via update(Window&).
#pragma once

#include <string>

struct SDL_Renderer;

namespace sdlw {

class Window;
class Font;

class TextBox {
public:
    struct Style {
        unsigned char bg[3]            = { 38,  38,  46 };
        unsigned char bgFocused[3]     = { 46,  46,  58 };
        unsigned char border[3]        = { 90,  90, 110 };
        unsigned char borderFocused[3] = { 120, 170, 235 };
        unsigned char text[3]          = { 235, 235, 240 };
        unsigned char placeholder[3]   = { 130, 130, 145 };
        unsigned char caret[3]         = { 235, 235, 240 };
    };

    TextBox() = default;
    TextBox(float x, float y, float w, float h);

    void setRect(float x, float y, float w, float h);
    void setText(std::string text);
    void setPlaceholder(std::string text);
    void setFocused(bool focused, Window& win);
    Style& style();

    const std::string& text() const { return text_; }
    bool focused() const { return focused_; }

    // Handle focus (via mouse), typed text, and editing keys for this frame.
    // Call once per frame after Window::pumpEvents().
    void update(Window& win);

    // Draw the field and its contents.
    void draw(SDL_Renderer* renderer, Font& font);

private:
    std::string text_;
    std::string placeholder_;
    float x_ = 0, y_ = 0, w_ = 0, h_ = 0;
    Style  style_;
    std::size_t caret_ = 0;   // caret position, in bytes into text_
    float  scroll_ = 0;       // horizontal scroll offset in pixels
    bool   focused_ = false;
    bool   wasDown_ = false;  // mouse-left state last frame
};

} // namespace sdlw
