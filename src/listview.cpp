#include "sdlw/listview.h"
#include "sdlw/window.h"
#include "sdlw/font.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <utility>

namespace sdlw {

ListView::ListView(float x, float y, float w, float h) : x_(x), y_(y), w_(w), h_(h) {}

void ListView::setRect(float x, float y, float w, float h) { x_ = x; y_ = y; w_ = w; h_ = h; }
void ListView::setColumns(std::vector<Column> cols) { columns_ = std::move(cols); }
void ListView::addColumn(std::string title, float width, Align align) {
    columns_.push_back({ std::move(title), width, align });
}
void ListView::setRows(std::vector<Row> rows) {
    rows_ = std::move(rows);
    if (selected_ >= rowCount()) selected_ = rowCount() - 1;
    scroll_ = 0;
}
void ListView::addRow(Row cells) { rows_.push_back(std::move(cells)); }
void ListView::clear() { rows_.clear(); selected_ = -1; hover_ = -1; scroll_ = 0; }

void ListView::setSelected(int index) {
    selected_ = (index >= 0 && index < rowCount()) ? index : -1;
}
const ListView::Row* ListView::selectedRow() const {
    return (selected_ >= 0 && selected_ < rowCount()) ? &rows_[selected_] : nullptr;
}

int   ListView::rowHeight(Font& font) const { return font.lineHeight() + rowPad_; }
float ListView::headerHeight(Font& font) const { return showHeader_ ? float(font.lineHeight() + 6) : 0.0f; }

float ListView::columnX(int i) const {
    float cx = x_;
    for (int c = 0; c < i && c < int(columns_.size()); ++c) cx += columns_[c].width;
    return cx;
}

float ListView::maxScroll(Font& font) const {
    float bodyH = h_ - headerHeight(font) - 2;
    return std::max(0.0f, float(rowCount() * rowHeight(font)) - bodyH);
}

void ListView::syncBar(Font& font) {
    float hH = headerHeight(font);
    float bodyTop = y_ + hH + 1;
    float bodyH = h_ - hH - 2;
    bar_.setRect(x_ + w_ - 8.0f, bodyTop, 7.0f, bodyH);
    bar_.setRange(float(rowCount() * rowHeight(font)), bodyH);
    bar_.style().thumb[0] = style_.scrollThumb[0];
    bar_.style().thumb[1] = style_.scrollThumb[1];
    bar_.style().thumb[2] = style_.scrollThumb[2];
    bar_.setValue(scroll_);
}

void ListView::scrollToSelected(Font& font) {
    if (selected_ < 0) return;
    int rh = rowHeight(font);
    float bodyH = h_ - headerHeight(font) - 2;
    float top = float(selected_ * rh), bottom = top + rh;
    if (top < scroll_)                 scroll_ = top;
    else if (bottom > scroll_ + bodyH) scroll_ = bottom - bodyH;
}

bool ListView::update(Window& win, Font& font) {
    activated_ = false;
    int rh = rowHeight(font);
    float hH = headerHeight(font);
    float bodyTop = y_ + hH + 1;
    float bodyH = h_ - hH - 2;
    int prev = selected_;

    float mx = win.mouseX(), my = win.mouseY();
    bool inBody = (mx >= x_ && mx < x_ + w_ && my >= bodyTop && my < bodyTop + bodyH);
    bool inWidget = (mx >= x_ && mx < x_ + w_ && my >= y_ && my < y_ + h_);

    if (inBody && win.mouseWheel() != 0) scroll_ -= win.mouseWheel() * rh * 3.0f;

    syncBar(font);
    bool overBar = bar_.needed() && bar_.hit(mx, my);

    hover_ = -1;
    if (inBody && !overBar) {
        int row = int((my - bodyTop + scroll_) / rh);
        if (row >= 0 && row < rowCount()) hover_ = row;
    }

    if (win.mousePressed()) {
        focused_ = inWidget;
        if (!overBar && inBody && hover_ >= 0) {
            selected_ = hover_;
            if (win.mouseClicks() >= 2) activated_ = true;
        }
    }

    bar_.update(win);
    scroll_ = bar_.value();

    if (focused_ && rowCount() > 0) {
        if (win.keyPressed(Key::Up))   selected_ = (selected_ <= 0) ? 0 : selected_ - 1;
        if (win.keyPressed(Key::Down)) selected_ = (selected_ < 0) ? 0 : std::min(selected_ + 1, rowCount() - 1);
        if (win.keyPressed(Key::Home)) selected_ = 0;
        if (win.keyPressed(Key::End))  selected_ = rowCount() - 1;
        int page = std::max(1, int(bodyH / rh));
        if (win.keyPressed(Key::PageUp))   selected_ = std::max(0, (selected_ < 0 ? 0 : selected_) - page);
        if (win.keyPressed(Key::PageDown)) selected_ = std::min(rowCount() - 1, (selected_ < 0 ? 0 : selected_) + page);
        if (win.keyPressed(Key::Enter) && selected_ >= 0) activated_ = true;
        if (selected_ != prev) scrollToSelected(font);
    }

    scroll_ = std::clamp(scroll_, 0.0f, maxScroll(font));
    return selected_ != prev;
}

void ListView::draw(SDL_Renderer* renderer, Font& font) {
    const int rh = rowHeight(font);
    const int lineH = font.lineHeight();
    const float hH = headerHeight(font);
    const float bodyTop = y_ + hH + 1;
    const float bodyH = h_ - hH - 2;
    const float cellPad = 8.0f;

    SDL_FRect rect{ x_, y_, w_, h_ };
    SDL_SetRenderDrawColor(renderer, style_.bg[0], style_.bg[1], style_.bg[2], 255);
    SDL_RenderFillRect(renderer, &rect);

    auto drawCell = [&](const std::string& text, const Column& col, float colX,
                        float rowY, float rowH, const unsigned char* color) {
        SDL_Rect clip{ int(colX), int(std::max(rowY, y_)), int(col.width),
                       int(std::min(rowY + rowH, y_ + h_) - std::max(rowY, y_)) };
        SDL_SetRenderClipRect(renderer, &clip);
        int tw = 0, th = 0;
        font.measure(text, &tw, &th);
        float tx = (col.align == Align::Right) ? colX + col.width - tw - cellPad : colX + cellPad;
        font.draw(text, tx, rowY + (rowH - lineH) * 0.5f, color[0], color[1], color[2]);
    };

    // Header (fixed).
    if (showHeader_) {
        SDL_FRect hdr{ x_, y_, w_, hH };
        SDL_SetRenderDrawColor(renderer, style_.headerBg[0], style_.headerBg[1], style_.headerBg[2], 255);
        SDL_RenderFillRect(renderer, &hdr);
        for (int c = 0; c < int(columns_.size()); ++c)
            drawCell(columns_[c].title, columns_[c], columnX(c), y_, hH, style_.headerText);
        SDL_SetRenderClipRect(renderer, nullptr);
        SDL_SetRenderDrawColor(renderer, style_.border[0], style_.border[1], style_.border[2], 255);
        SDL_RenderLine(renderer, x_, y_ + hH, x_ + w_, y_ + hH);
    }

    // Body rows (clipped, only the visible range).
    SDL_Rect bodyClip{ int(x_) + 1, int(bodyTop), int(w_) - 2, int(bodyH) };
    SDL_SetRenderClipRect(renderer, &bodyClip);
    int first = std::max(0, int(scroll_ / rh));
    int last  = std::min(rowCount() - 1, int((scroll_ + bodyH) / rh));
    for (int i = first; i <= last; ++i) {
        // Reset to the body clip: drawCell narrows the clip per cell, so the
        // next row's highlight fill must restore the full-width viewport first.
        SDL_SetRenderClipRect(renderer, &bodyClip);
        float rowY = bodyTop + i * rh - scroll_;
        SDL_FRect row{ x_ + 1, rowY, w_ - 2, float(rh) };
        const unsigned char* txt = style_.item;
        if (i == selected_) {
            SDL_SetRenderDrawColor(renderer, style_.selectedBg[0], style_.selectedBg[1], style_.selectedBg[2], 255);
            SDL_RenderFillRect(renderer, &row);
            txt = style_.selectedText;
        } else if (i == hover_) {
            SDL_SetRenderDrawColor(renderer, style_.hoverBg[0], style_.hoverBg[1], style_.hoverBg[2], 255);
            SDL_RenderFillRect(renderer, &row);
        }
        for (int c = 0; c < int(columns_.size()); ++c) {
            const std::string& cell = (c < int(rows_[i].size())) ? rows_[i][c] : std::string();
            drawCell(cell, columns_[c], columnX(c), rowY, float(rh), txt);
        }
    }
    SDL_SetRenderClipRect(renderer, nullptr);

    // Vertical gridlines between columns (within the body).
    SDL_SetRenderDrawColor(renderer, style_.gridline[0], style_.gridline[1], style_.gridline[2], 255);
    for (int c = 1; c < int(columns_.size()); ++c) {
        float gx = columnX(c);
        if (gx > x_ && gx < x_ + w_) SDL_RenderLine(renderer, gx, y_ + 1, gx, y_ + h_ - 1);
    }

    SDL_SetRenderDrawColor(renderer, style_.border[0], style_.border[1], style_.border[2], 255);
    SDL_RenderRect(renderer, &rect);

    syncBar(font);
    bar_.draw(renderer);
}

} // namespace sdlw
