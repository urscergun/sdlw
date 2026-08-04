#include "sdlw/select.h"
#include "sdlw/window.h"
#include "sdlw/font.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <utility>

namespace sdlw {

Select::Select(float x, float y, float w, float h) { setRect(x, y, w, h); }

void Select::setRect(float x, float y, float w, float h) { x_ = x; y_ = y; w_ = w; h_ = h; }
void Select::setItems(std::vector<std::string> items) {
    items_ = std::move(items);
    list_.setItems(items_);
    if (selected_ >= int(items_.size())) selected_ = int(items_.size()) - 1;
}
void Select::setSelected(int index) {
    selected_ = (index >= 0 && index < int(items_.size())) ? index : -1;
    list_.setSelected(selected_);
}
void Select::setPlaceholder(std::string text) { placeholder_ = std::move(text); }
void Select::setMaxVisibleRows(int rows) { maxVisibleRows_ = std::max(1, rows); }

const std::string* Select::selectedItem() const {
    return (selected_ >= 0 && selected_ < int(items_.size())) ? &items_[selected_] : nullptr;
}

void Select::layout(Font& font) {
    int rows = std::clamp(list_.count(), 1, maxVisibleRows_);
    float listH = float(rows * list_.rowHeight(font)) + 2.0f;
    list_.setRect(x_, y_ + h_ + 2, w_, listH);
}

bool Select::update(Window& win, Font& font) {
    layout(font);
    bool committed = false;

    float mx = win.mouseX(), my = win.mouseY();
    bool overField = (mx >= x_ && mx < x_ + w_ && my >= y_ && my < y_ + h_);
    int rows = std::clamp(list_.count(), 1, maxVisibleRows_);
    float listTop = y_ + h_ + 2;
    float listH = float(rows * list_.rowHeight(font)) + 2.0f;
    bool overList = open_ && (mx >= x_ && mx < x_ + w_ && my >= listTop && my < listTop + listH);

    if (win.mousePressed()) {
        if (overField) {
            open_ = !open_;
            if (open_) { list_.setSelected(selected_); list_.select(selected_, font); }
        } else if (!overList) {
            open_ = false;
        }
    }

    // Keyboard: open when focused via Tab, using Down/Space/Enter.
    if (focused_ && !open_ &&
        (win.keyPressed(Key::Down) || win.keyPressed(Key::Space) || win.keyPressed(Key::Enter))) {
        open_ = true;
        list_.setSelected(selected_);
        list_.select(selected_, font);
    }

    if (open_) {
        list_.update(win, font);
        if (list_.itemClicked()) {
            selected_ = list_.selected();
            open_ = false;
            committed = true;
        }
        if (win.keyPressed(Key::Down)) {
            int i = list_.selected();
            list_.select(i < 0 ? 0 : std::min(i + 1, list_.count() - 1), font);
        }
        if (win.keyPressed(Key::Up)) {
            int i = list_.selected();
            list_.select(i <= 0 ? 0 : i - 1, font);
        }
        if (win.keyPressed(Key::Enter)) {
            selected_ = list_.selected();
            open_ = false;
            committed = (selected_ >= 0);
        }
    }
    return committed;
}

void Select::draw(SDL_Renderer* renderer, Font& font) {
    layout(font);

    SDL_FRect field{ x_, y_, w_, h_ };
    SDL_SetRenderDrawColor(renderer, style_.bg[0], style_.bg[1], style_.bg[2], 255);
    SDL_RenderFillRect(renderer, &field);
    SDL_SetRenderDrawColor(renderer, style_.border[0], style_.border[1], style_.border[2], 255);
    SDL_RenderRect(renderer, &field);

    const std::string* sel = selectedItem();
    float ty = y_ + (h_ - font.lineHeight()) * 0.5f;
    if (sel) {
        font.draw(*sel, x_ + 8, ty, style_.text[0], style_.text[1], style_.text[2]);
    } else if (!placeholder_.empty()) {
        font.draw(placeholder_, x_ + 8, ty,
                  style_.placeholder[0], style_.placeholder[1], style_.placeholder[2]);
    }

    // Chevron (down when closed, up when open).
    float cx = x_ + w_ - h_ * 0.5f, cy = y_ + h_ * 0.5f, s = 4.0f;
    SDL_SetRenderDrawColor(renderer, style_.arrow[0], style_.arrow[1], style_.arrow[2], 255);
    if (open_) {
        SDL_RenderLine(renderer, cx - s, cy + s * 0.5f, cx, cy - s * 0.5f);
        SDL_RenderLine(renderer, cx + s, cy + s * 0.5f, cx, cy - s * 0.5f);
    } else {
        SDL_RenderLine(renderer, cx - s, cy - s * 0.5f, cx, cy + s * 0.5f);
        SDL_RenderLine(renderer, cx + s, cy - s * 0.5f, cx, cy + s * 0.5f);
    }

    if (open_ && list_.count() > 0) list_.draw(renderer, font);
}

} // namespace sdlw
