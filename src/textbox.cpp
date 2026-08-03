#include "sdlw/textbox.h"
#include "sdlw/window.h"
#include "sdlw/font.h"

#include <SDL3/SDL.h>

#include <utility>

namespace sdlw {

namespace {
// UTF-8 aware caret movement over a byte buffer.
std::size_t prevCharStart(const std::string& s, std::size_t i) {
    if (i == 0) return 0;
    --i;
    while (i > 0 && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80) --i;
    return i;
}
std::size_t nextCharStart(const std::string& s, std::size_t i) {
    std::size_t n = s.size();
    if (i >= n) return n;
    ++i;
    while (i < n && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80) ++i;
    return i;
}
} // namespace

TextBox::TextBox(float x, float y, float w, float h) : x_(x), y_(y), w_(w), h_(h) {}

void TextBox::setRect(float x, float y, float w, float h) { x_ = x; y_ = y; w_ = w; h_ = h; }
void TextBox::setText(std::string text) { text_ = std::move(text); caret_ = text_.size(); }
void TextBox::setPlaceholder(std::string text) { placeholder_ = std::move(text); }
TextBox::Style& TextBox::style() { return style_; }

void TextBox::setFocused(bool focused, Window& win) {
    if (focused == focused_) return;
    focused_ = focused;
    if (focused_) win.startTextInput();
    else          win.stopTextInput();
}

void TextBox::update(Window& win) {
    // Focus follows mouse clicks.
    float mx = 0, my = 0;
    bool down = (SDL_GetMouseState(&mx, &my) & SDL_BUTTON_LMASK) != 0;
    bool inside = (mx >= x_ && mx < x_ + w_ && my >= y_ && my < y_ + h_);
    if (down && !wasDown_) {
        setFocused(inside, win);
        if (inside) caret_ = text_.size(); // caret to end on focus
    }
    wasDown_ = down;

    if (!focused_) return;

    // Insert typed text at the caret.
    const char* typed = win.textInput();
    if (typed && *typed) {
        std::string in(typed);
        text_.insert(caret_, in);
        caret_ += in.size();
    }

    // Editing keys.
    if (win.keyPressed(Key::Backspace) && caret_ > 0) {
        std::size_t s = prevCharStart(text_, caret_);
        text_.erase(s, caret_ - s);
        caret_ = s;
    }
    if (win.keyPressed(Key::Delete) && caret_ < text_.size()) {
        std::size_t e = nextCharStart(text_, caret_);
        text_.erase(caret_, e - caret_);
    }
    if (win.keyPressed(Key::Left))  caret_ = prevCharStart(text_, caret_);
    if (win.keyPressed(Key::Right)) caret_ = nextCharStart(text_, caret_);
    if (win.keyPressed(Key::Home))  caret_ = 0;
    if (win.keyPressed(Key::End))   caret_ = text_.size();
    if (caret_ > text_.size()) caret_ = text_.size();
}

void TextBox::draw(SDL_Renderer* renderer, Font& font) {
    SDL_FRect rect{ x_, y_, w_, h_ };

    const unsigned char* bg = focused_ ? style_.bgFocused : style_.bg;
    SDL_SetRenderDrawColor(renderer, bg[0], bg[1], bg[2], 255);
    SDL_RenderFillRect(renderer, &rect);

    const unsigned char* bd = focused_ ? style_.borderFocused : style_.border;
    SDL_SetRenderDrawColor(renderer, bd[0], bd[1], bd[2], 255);
    SDL_RenderRect(renderer, &rect);

    const float pad = 6.0f;
    const float innerW = w_ - 2 * pad;
    const int   lineH = font.lineHeight();
    const float ty = y_ + (h_ - lineH) * 0.5f;

    // Clip drawing to the inner area so long text/caret don't spill out.
    SDL_Rect clip{ int(x_) + 1, int(y_) + 1, int(w_) - 2, int(h_) - 2 };
    SDL_SetRenderClipRect(renderer, &clip);

    if (text_.empty() && !focused_ && !placeholder_.empty()) {
        font.draw(placeholder_, x_ + pad, ty,
                  style_.placeholder[0], style_.placeholder[1], style_.placeholder[2]);
    } else {
        // Keep the caret within the visible area by scrolling horizontally.
        int caretW = 0, dummy = 0, fullW = 0;
        font.measure(text_.substr(0, caret_), &caretW, &dummy);
        font.measure(text_, &fullW, &dummy);
        if (fullW <= int(innerW))            scroll_ = 0;
        else if (caretW - scroll_ > innerW)  scroll_ = caretW - innerW;
        else if (caretW - scroll_ < 0)       scroll_ = float(caretW);

        font.draw(text_, x_ + pad - scroll_, ty,
                  style_.text[0], style_.text[1], style_.text[2]);

        // Blinking caret (~1.7 Hz) while focused.
        if (focused_ && (SDL_GetTicks() / 500) % 2 == 0) {
            SDL_FRect caret{ x_ + pad - scroll_ + caretW, ty, 2.0f, float(lineH) };
            SDL_SetRenderDrawColor(renderer, style_.caret[0], style_.caret[1], style_.caret[2], 255);
            SDL_RenderFillRect(renderer, &caret);
        }
    }

    SDL_SetRenderClipRect(renderer, nullptr);
}

} // namespace sdlw
