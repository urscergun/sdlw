#include "sdlw/combobox.h"
#include "sdlw/window.h"
#include "sdlw/font.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cctype>
#include <utility>

namespace sdlw {

namespace {
std::string toLower(std::string s) {
    for (char& c : s) c = char(std::tolower(static_cast<unsigned char>(c)));
    return s;
}
bool containsCI(const std::string& hay, const std::string& needle) {
    if (needle.empty()) return true;
    return toLower(hay).find(toLower(needle)) != std::string::npos;
}
} // namespace

ComboBox::ComboBox(float x, float y, float w, float h) { setRect(x, y, w, h); }

void ComboBox::setRect(float x, float y, float w, float h) {
    x_ = x; y_ = y; w_ = w; h_ = h; arrowW_ = h;
}
void ComboBox::setItems(std::vector<std::string> items) {
    items_ = std::move(items);
    refilter();
}
void ComboBox::setText(std::string text) {
    field_.setText(text);
    committed_ = true;
    lastText_ = std::move(text);
    refilter();
}
void ComboBox::setMaxVisibleRows(int rows) { maxVisibleRows_ = std::max(1, rows); }
const std::string& ComboBox::text() const { return field_.text(); }

void ComboBox::layout(Font& font) {
    field_.setRect(x_, y_, w_ - arrowW_, h_);
    int rows = std::clamp(list_.count(), 1, maxVisibleRows_);
    float listH = float(rows * list_.rowHeight(font)) + 2.0f;
    list_.setRect(x_, y_ + h_ + 2, w_, listH);
}

void ComboBox::refilter() {
    const std::string& t = field_.text();
    std::vector<std::string> filtered;
    int sel = -1;
    if (committed_) {
        // A value is chosen: show the whole list so the user can browse,
        // highlighting the current value if it's present.
        filtered = items_;
        for (int i = 0; i < int(items_.size()); ++i)
            if (items_[i] == t) { sel = i; break; }
    } else {
        // Typing: filter to matching items.
        for (const std::string& it : items_)
            if (containsCI(it, t)) filtered.push_back(it);
        sel = filtered.empty() ? -1 : 0;
    }
    list_.setItems(std::move(filtered));
    list_.setSelected(sel);
    lastText_ = t;
}

void ComboBox::commit(const std::string& item, Window& win) {
    field_.setText(item);
    committed_ = true;
    lastText_ = item;
    open_ = false;
    field_.setFocused(false, win);
}

bool ComboBox::update(Window& win, Font& font) {
    layout(font);
    bool committed = false;

    float mx = 0, my = 0;
    SDL_GetMouseState(&mx, &my);
    bool overArrow = (mx >= x_ + w_ - arrowW_ && mx < x_ + w_ && my >= y_ && my < y_ + h_);
    bool overField = (mx >= x_ && mx < x_ + w_ - arrowW_ && my >= y_ && my < y_ + h_);

    int rows = std::clamp(list_.count(), 1, maxVisibleRows_);
    float listTop = y_ + h_ + 2;
    float listH = float(rows * list_.rowHeight(font)) + 2.0f;
    bool overList = open_ && (mx >= x_ && mx < x_ + w_ && my >= listTop && my < listTop + listH);

    // Open the popup: refresh the list and scroll the current value into view.
    auto openPopup = [&]() {
        open_ = true;
        refilter();
        int s = list_.selected();
        if (s >= 0) list_.select(s, font);
    };

    // Press outside everything closes the popup; the arrow toggles it.
    if (win.mousePressed()) {
        if (overArrow) {
            if (open_) open_ = false;
            else { field_.setFocused(true, win); openPopup(); }
        } else if (overField) {
            if (!open_) openPopup();
        } else if (!overList) {
            open_ = false;
        }
    }

    // Field editing (typing, caret, selection). Any text edit leaves "browse"
    // mode and filters the list.
    field_.update(win, font);
    if (field_.text() != lastText_) {
        committed_ = false;
        open_ = true;
        refilter();
    }

    // Open on Down when focused but closed.
    if (field_.focused() && !open_ && win.keyPressed(Key::Down)) {
        openPopup();
    }

    if (open_) {
        // Mouse interaction with the popup; a click on any item commits it
        // (even if it was already the selected/highlighted row).
        list_.update(win, font);
        if (list_.itemClicked()) {
            if (const std::string* s = list_.selectedItem()) { commit(*s, win); committed = true; }
        }
        // Keyboard: Up/Down move the highlight, Enter commits it.
        if (win.keyPressed(Key::Down)) {
            int i = list_.selected();
            list_.select(i < 0 ? 0 : std::min(i + 1, list_.count() - 1), font);
        }
        if (win.keyPressed(Key::Up)) {
            int i = list_.selected();
            list_.select(i <= 0 ? 0 : i - 1, font);
        }
        if (win.keyPressed(Key::Enter)) {
            if (const std::string* s = list_.selectedItem()) { commit(*s, win); committed = true; }
        }
    }

    return committed;
}

void ComboBox::draw(SDL_Renderer* renderer, Font& font) {
    layout(font);

    field_.draw(renderer, font);

    // Arrow button background.
    SDL_FRect ab{ x_ + w_ - arrowW_, y_, arrowW_, h_ };
    SDL_SetRenderDrawColor(renderer, style_.arrowBg[0], style_.arrowBg[1], style_.arrowBg[2], 255);
    SDL_RenderFillRect(renderer, &ab);

    // Chevron (points down when closed, up when open) drawn as two lines.
    float cx = x_ + w_ - arrowW_ * 0.5f;
    float cy = y_ + h_ * 0.5f;
    float s = 4.0f;
    SDL_SetRenderDrawColor(renderer, style_.arrow[0], style_.arrow[1], style_.arrow[2], 255);
    if (open_) {
        SDL_RenderLine(renderer, cx - s, cy + s * 0.5f, cx, cy - s * 0.5f);
        SDL_RenderLine(renderer, cx + s, cy + s * 0.5f, cx, cy - s * 0.5f);
    } else {
        SDL_RenderLine(renderer, cx - s, cy - s * 0.5f, cx, cy + s * 0.5f);
        SDL_RenderLine(renderer, cx + s, cy - s * 0.5f, cx, cy + s * 0.5f);
    }

    if (open_ && list_.count() > 0) {
        list_.draw(renderer, font);
    }
}

} // namespace sdlw
