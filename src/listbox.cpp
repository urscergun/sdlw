#include "sdlw/listbox.h"
#include "sdlw/window.h"
#include "sdlw/font.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <utility>

namespace sdlw {

ListBox::ListBox(float x, float y, float w, float h) : x_(x), y_(y), w_(w), h_(h) {}

void ListBox::setRect(float x, float y, float w, float h) { x_ = x; y_ = y; w_ = w; h_ = h; }
void ListBox::setItems(std::vector<std::string> items) {
    items_ = std::move(items);
    if (selected_ >= count()) selected_ = count() - 1;
    scroll_ = 0;
}
void ListBox::addItem(std::string item) { items_.push_back(std::move(item)); }
void ListBox::clear() { items_.clear(); selected_ = -1; hover_ = -1; scroll_ = 0; }
ListBox::Style& ListBox::style() { return style_; }

void ListBox::setSelected(int index) {
    selected_ = (index >= 0 && index < count()) ? index : -1;
}
const std::string* ListBox::selectedItem() const {
    return (selected_ >= 0 && selected_ < count()) ? &items_[selected_] : nullptr;
}

int ListBox::rowHeight(Font& font) const { return font.lineHeight() + rowPad_; }

float ListBox::maxScroll(Font& font) const {
    float contentH = float(count() * rowHeight(font));
    float viewH = h_ - 2;
    return std::max(0.0f, contentH - viewH);
}

void ListBox::scrollToSelected(Font& font) {
    if (selected_ < 0) return;
    int rh = rowHeight(font);
    float top = float(selected_ * rh);
    float bottom = top + rh;
    float viewH = h_ - 2;
    if (top < scroll_)              scroll_ = top;
    else if (bottom > scroll_ + viewH) scroll_ = bottom - viewH;
}

bool ListBox::update(Window& win, Font& font) {
    int rh = rowHeight(font);
    int prevSelected = selected_;

    float mx = 0, my = 0;
    SDL_GetMouseState(&mx, &my);
    bool inside = (mx >= x_ && mx < x_ + w_ && my >= y_ && my < y_ + h_);

    // Hover + which row is under the cursor.
    hover_ = -1;
    if (inside) {
        int row = int((my - y_ - 1 + scroll_) / rh);
        if (row >= 0 && row < count()) hover_ = row;
    }

    // Wheel scrolling (3 rows per notch) when the cursor is over the list.
    if (inside && win.mouseWheel() != 0) {
        scroll_ -= win.mouseWheel() * rh * 3.0f;
    }

    // Click to focus + select.
    if (win.mousePressed()) {
        focused_ = inside;
        if (inside && hover_ >= 0) selected_ = hover_;
    }

    // Keyboard navigation while focused.
    if (focused_ && count() > 0) {
        if (win.keyPressed(Key::Up))   selected_ = (selected_ <= 0) ? 0 : selected_ - 1;
        if (win.keyPressed(Key::Down)) selected_ = (selected_ < 0) ? 0
                                                  : std::min(selected_ + 1, count() - 1);
        if (win.keyPressed(Key::Home)) selected_ = 0;
        if (win.keyPressed(Key::End))  selected_ = count() - 1;
        // Page = number of fully visible rows (at least 1).
        int page = std::max(1, int((h_ - 2) / rh));
        if (win.keyPressed(Key::PageUp))
            selected_ = std::max(0, (selected_ < 0 ? 0 : selected_) - page);
        if (win.keyPressed(Key::PageDown))
            selected_ = std::min(count() - 1, (selected_ < 0 ? 0 : selected_) + page);
        if (selected_ != prevSelected) scrollToSelected(font);
    }

    // Keep scroll in range.
    scroll_ = std::clamp(scroll_, 0.0f, maxScroll(font));

    return selected_ != prevSelected;
}

void ListBox::draw(SDL_Renderer* renderer, Font& font) {
    SDL_FRect rect{ x_, y_, w_, h_ };
    SDL_SetRenderDrawColor(renderer, style_.bg[0], style_.bg[1], style_.bg[2], 255);
    SDL_RenderFillRect(renderer, &rect);

    const int rh = rowHeight(font);
    const int lineH = font.lineHeight();
    const float pad = 8.0f;

    SDL_Rect clip{ int(x_) + 1, int(y_) + 1, int(w_) - 2, int(h_) - 2 };
    SDL_SetRenderClipRect(renderer, &clip);

    // Only draw the visible row range.
    int first = std::max(0, int(scroll_ / rh));
    int last  = std::min(count() - 1, int((scroll_ + h_) / rh));
    for (int i = first; i <= last; ++i) {
        float rowY = y_ + 1 + i * rh - scroll_;
        SDL_FRect row{ x_ + 1, rowY, w_ - 2, float(rh) };
        const unsigned char* txtColor = style_.item;
        if (i == selected_) {
            SDL_SetRenderDrawColor(renderer, style_.selectedBg[0], style_.selectedBg[1], style_.selectedBg[2], 255);
            SDL_RenderFillRect(renderer, &row);
            txtColor = style_.selectedText;
        } else if (i == hover_) {
            SDL_SetRenderDrawColor(renderer, style_.hoverBg[0], style_.hoverBg[1], style_.hoverBg[2], 255);
            SDL_RenderFillRect(renderer, &row);
        }
        font.draw(items_[i], x_ + pad, rowY + (rh - lineH) * 0.5f,
                  txtColor[0], txtColor[1], txtColor[2]);
    }

    SDL_SetRenderClipRect(renderer, nullptr);

    // Border.
    SDL_SetRenderDrawColor(renderer, style_.border[0], style_.border[1], style_.border[2], 255);
    SDL_RenderRect(renderer, &rect);

    // Scrollbar thumb (only when content overflows).
    float contentH = float(count() * rh);
    float viewH = h_ - 2;
    if (contentH > viewH) {
        float trackH = viewH;
        float thumbH = std::max(20.0f, trackH * (viewH / contentH));
        float maxS = maxScroll(font);
        float t = (maxS > 0) ? (scroll_ / maxS) : 0.0f;
        float thumbY = y_ + 1 + t * (trackH - thumbH);
        SDL_FRect thumb{ x_ + w_ - 7, thumbY, 5.0f, thumbH };
        SDL_SetRenderDrawColor(renderer, style_.scrollThumb[0], style_.scrollThumb[1], style_.scrollThumb[2], 255);
        SDL_RenderFillRect(renderer, &thumb);
    }
}

} // namespace sdlw
