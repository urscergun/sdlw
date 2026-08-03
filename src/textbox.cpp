#include "sdlw/textbox.h"
#include "sdlw/window.h"
#include "sdlw/font.h"

#include <SDL3/SDL.h>

#include <algorithm>
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
// Pixel width of s[0..idx) at the current font.
int widthTo(Font& font, const std::string& s, std::size_t idx) {
    int w = 0, h = 0;
    font.measure(s.substr(0, idx), &w, &h);
    return w;
}
// Byte index whose caret slot is nearest to pixel offset targetX (text-relative).
std::size_t indexFromX(Font& font, const std::string& s, float targetX) {
    if (targetX <= 0) return 0;
    std::size_t i = 0;
    float acc = 0;
    while (i < s.size()) {
        std::size_t j = nextCharStart(s, i);
        int w = 0, h = 0;
        font.measure(s.substr(i, j - i), &w, &h);
        if (targetX < acc + w * 0.5f) return i;
        acc += w;
        i = j;
    }
    return s.size();
}
} // namespace

TextBox::TextBox(float x, float y, float w, float h) : x_(x), y_(y), w_(w), h_(h) {}

void TextBox::setRect(float x, float y, float w, float h) { x_ = x; y_ = y; w_ = w; h_ = h; }
void TextBox::setText(std::string text) { text_ = std::move(text); caret_ = sel_ = text_.size(); }
void TextBox::setPlaceholder(std::string text) { placeholder_ = std::move(text); }
TextBox::Style& TextBox::style() { return style_; }

std::string TextBox::selectedText() const {
    std::size_t lo = std::min(sel_, caret_), hi = std::max(sel_, caret_);
    return text_.substr(lo, hi - lo);
}

void TextBox::setFocused(bool focused, Window& win) {
    if (focused == focused_) return;
    focused_ = focused;
    if (focused_) win.startTextInput();
    else          win.stopTextInput();
}

void TextBox::update(Window& win, Font& font) {
    const float pad = 6.0f;
    bool shift = (SDL_GetModState() & SDL_KMOD_SHIFT) != 0;

    // --- Mouse: focus, click-to-place caret, drag-to-select ----------------
    float mx = 0, my = 0;
    bool down = (SDL_GetMouseState(&mx, &my) & SDL_BUTTON_LMASK) != 0;
    bool inside = (mx >= x_ && mx < x_ + w_ && my >= y_ && my < y_ + h_);
    float localX = mx - (x_ + pad) + scroll_;

    if (down && !wasDown_) {                 // press edge
        setFocused(inside, win);
        if (inside) {
            std::size_t hit = indexFromX(font, text_, localX);
            caret_ = hit;
            if (!shift) sel_ = hit;          // shift-click extends existing selection
            dragging_ = true;
        }
    } else if (down && dragging_ && focused_) { // dragging extends the caret end
        caret_ = indexFromX(font, text_, localX);
    } else if (!down) {
        dragging_ = false;
    }
    wasDown_ = down;

    if (!focused_) return;

    auto deleteSelection = [&]() {
        std::size_t lo = std::min(sel_, caret_), hi = std::max(sel_, caret_);
        text_.erase(lo, hi - lo);
        caret_ = sel_ = lo;
    };

    // --- Clipboard shortcuts ----------------------------------------------
    if (win.keyPressed(Key::Copy) && hasSelection()) {
        SDL_SetClipboardText(selectedText().c_str());
    }
    if (win.keyPressed(Key::Cut) && hasSelection()) {
        SDL_SetClipboardText(selectedText().c_str());
        deleteSelection();
    }
    if (win.keyPressed(Key::Paste)) {
        if (char* clip = SDL_GetClipboardText()) {
            std::string in(clip);
            SDL_free(clip);
            in.erase(std::remove(in.begin(), in.end(), '\n'), in.end()); // single line
            in.erase(std::remove(in.begin(), in.end(), '\r'), in.end());
            if (hasSelection()) deleteSelection();
            text_.insert(caret_, in);
            caret_ += in.size();
            sel_ = caret_;
        }
    }
    if (win.keyPressed(Key::SelectAll)) {
        sel_ = 0;
        caret_ = text_.size();
    }

    // --- Typed text (replaces any selection) -------------------------------
    const char* typed = win.textInput();
    if (typed && *typed) {
        if (hasSelection()) deleteSelection();
        std::string in(typed);
        text_.insert(caret_, in);
        caret_ += in.size();
        sel_ = caret_;
    }

    // --- Editing keys ------------------------------------------------------
    if (win.keyPressed(Key::Backspace)) {
        if (hasSelection())      deleteSelection();
        else if (caret_ > 0) {
            std::size_t s = prevCharStart(text_, caret_);
            text_.erase(s, caret_ - s);
            caret_ = sel_ = s;
        }
    }
    if (win.keyPressed(Key::Delete)) {
        if (hasSelection())              deleteSelection();
        else if (caret_ < text_.size()) {
            std::size_t e = nextCharStart(text_, caret_);
            text_.erase(caret_, e - caret_);
            sel_ = caret_;
        }
    }
    if (win.keyPressed(Key::Left)) {
        if (hasSelection() && !shift) caret_ = std::min(sel_, caret_);
        else                          caret_ = prevCharStart(text_, caret_);
        if (!shift) sel_ = caret_;
    }
    if (win.keyPressed(Key::Right)) {
        if (hasSelection() && !shift) caret_ = std::max(sel_, caret_);
        else                          caret_ = nextCharStart(text_, caret_);
        if (!shift) sel_ = caret_;
    }
    if (win.keyPressed(Key::Home)) { caret_ = 0; if (!shift) sel_ = 0; }
    if (win.keyPressed(Key::End))  { caret_ = text_.size(); if (!shift) sel_ = caret_; }

    if (caret_ > text_.size()) caret_ = text_.size();
    if (sel_ > text_.size())   sel_ = text_.size();
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
    const float originX = x_ + pad;

    SDL_Rect clip{ int(x_) + 1, int(y_) + 1, int(w_) - 2, int(h_) - 2 };
    SDL_SetRenderClipRect(renderer, &clip);

    if (text_.empty() && !focused_ && !placeholder_.empty()) {
        font.draw(placeholder_, originX, ty,
                  style_.placeholder[0], style_.placeholder[1], style_.placeholder[2]);
    } else {
        int caretW = widthTo(font, text_, caret_);
        int fullW  = widthTo(font, text_, text_.size());

        // Scroll so the caret stays visible.
        if (fullW <= int(innerW))            scroll_ = 0;
        else if (caretW - scroll_ > innerW)  scroll_ = caretW - innerW;
        else if (caretW - scroll_ < 0)       scroll_ = float(caretW);

        // Selection highlight (behind the text).
        if (hasSelection()) {
            std::size_t lo = std::min(sel_, caret_), hi = std::max(sel_, caret_);
            float xlo = originX - scroll_ + widthTo(font, text_, lo);
            float xhi = originX - scroll_ + widthTo(font, text_, hi);
            SDL_FRect hl{ xlo, ty, xhi - xlo, float(lineH) };
            SDL_SetRenderDrawColor(renderer, style_.selection[0], style_.selection[1], style_.selection[2], 255);
            SDL_RenderFillRect(renderer, &hl);
        }

        font.draw(text_, originX - scroll_, ty,
                  style_.text[0], style_.text[1], style_.text[2]);

        // Blinking caret (~1.7 Hz) while focused.
        if (focused_ && (SDL_GetTicks() / 500) % 2 == 0) {
            SDL_FRect caret{ originX - scroll_ + caretW, ty, 2.0f, float(lineH) };
            SDL_SetRenderDrawColor(renderer, style_.caret[0], style_.caret[1], style_.caret[2], 255);
            SDL_RenderFillRect(renderer, &caret);
        }
    }

    SDL_SetRenderClipRect(renderer, nullptr);
}

} // namespace sdlw
