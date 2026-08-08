#include "sdlw/listview.h"
#include "sdlw/window.h"
#include "sdlw/font.h"

#include "text_util.h" // detail::toLower for case-insensitive alpha sort

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstdlib>
#include <utility>

namespace sdlw {

ListView::ListView(float x, float y, float w, float h) : x_(x), y_(y), w_(w), h_(h) {}

void ListView::setRect(float x, float y, float w, float h) { x_ = x; y_ = y; w_ = w; h_ = h; }
void ListView::setColumns(std::vector<Column> cols) { columns_ = std::move(cols); }
void ListView::addColumn(std::string title, float width, Align align) {
    columns_.push_back({ std::move(title), width, align, SortType::Alpha, true });
}
void ListView::setRows(std::vector<Row> rows) {
    rows_ = std::move(rows);
    selected_ = -1;
    scroll_ = 0;
    rebuildOrder();
}
void ListView::addRow(Row cells) { rows_.push_back(std::move(cells)); rebuildOrder(); }
void ListView::clear() {
    rows_.clear(); order_.clear();
    selected_ = -1; hover_ = -1; scroll_ = 0;
    sortCol_ = -1; dir_ = SortDir::None;
}

void ListView::setSelected(int index) {
    selected_ = (index >= 0 && index < rowCount()) ? index : -1;
}
const ListView::Row* ListView::selectedRow() const { return rowAt(selected_); }
const ListView::Row* ListView::rowAt(int i) const {
    return (i >= 0 && i < int(order_.size())) ? &rows_[order_[i]] : nullptr;
}

std::string ListView::cellStr(int row, int col) const {
    if (row < 0 || row >= int(rows_.size())) return {};
    return (col >= 0 && col < int(rows_[row].size())) ? rows_[row][col] : std::string();
}

void ListView::rebuildOrder() {
    int n = int(rows_.size());
    // Remember which underlying row is selected so it stays selected after sort.
    int selUnderlying = (selected_ >= 0 && selected_ < int(order_.size())) ? order_[selected_] : -1;

    order_.resize(n);
    for (int i = 0; i < n; ++i) order_[i] = i;

    if (sortCol_ >= 0 && sortCol_ < int(columns_.size()) && dir_ != SortDir::None) {
        SortType st = columns_[sortCol_].sort;
        bool desc = (dir_ == SortDir::Descending);
        std::stable_sort(order_.begin(), order_.end(), [&](int a, int b) {
            int cmp;
            if (st == SortType::Numeric) {
                double da = std::strtod(cellStr(a, sortCol_).c_str(), nullptr);
                double db = std::strtod(cellStr(b, sortCol_).c_str(), nullptr);
                cmp = (da < db) ? -1 : (da > db ? 1 : 0);
            } else {
                cmp = detail::toLower(cellStr(a, sortCol_))
                          .compare(detail::toLower(cellStr(b, sortCol_)));
            }
            return desc ? cmp > 0 : cmp < 0;
        });
    }

    // Restore selection to the same underlying item's new display position.
    selected_ = -1;
    if (selUnderlying >= 0)
        for (int i = 0; i < n; ++i)
            if (order_[i] == selUnderlying) { selected_ = i; break; }
}

void ListView::sortBy(int column, SortDir dir) {
    sortCol_ = (dir == SortDir::None) ? -1 : column;
    dir_ = dir;
    rebuildOrder();
}

int   ListView::rowHeight(Font& font) const { return font.lineHeight() + rowPad_; }
float ListView::headerHeight(Font& font) const { return showHeader_ ? float(font.lineHeight() + 6) : 0.0f; }

float ListView::columnX(int i) const {
    float cx = x_;
    for (int c = 0; c < i && c < int(columns_.size()); ++c) cx += columns_[c].width;
    return cx;
}

int ListView::columnAtX(float mx) const {
    float cx = x_;
    for (int c = 0; c < int(columns_.size()); ++c) {
        if (mx >= cx && mx < cx + columns_[c].width) return c;
        cx += columns_[c].width;
    }
    return -1;
}

float ListView::contentWidth() const {
    float sum = 0;
    for (const Column& c : columns_) sum += c.width;
    return sum;
}

float ListView::maxScroll(Font& font) const {
    return std::max(0.0f, float(rowCount() * rowHeight(font)) - bodyH_);
}

// Decide which scrollbars are needed (two-pass, since each steals space from
// the other) and size both bars. Records the content viewport in viewW_/bodyH_.
void ListView::layoutBars(Font& font) {
    const float barW = 8.0f;
    float hH = headerHeight(font);
    float rowsH = float(rowCount() * rowHeight(font));
    float contentW = contentWidth();
    float fullBodyH = h_ - hH - 2;

    bool vN = rowsH > fullBodyH;
    bool hN = contentW > (w_ - 2 - (vN ? barW : 0));
    float bodyH = fullBodyH - (hN ? barW : 0);
    vN = rowsH > bodyH;
    float viewW = w_ - 2 - (vN ? barW : 0);
    hN = contentW > viewW;
    bodyH = fullBodyH - (hN ? barW : 0);

    viewW_ = viewW;
    bodyH_ = bodyH;
    float bodyTop = y_ + hH + 1;

    auto tint = [&](Scrollbar& b) {
        b.style().thumb[0] = style_.scrollThumb[0];
        b.style().thumb[1] = style_.scrollThumb[1];
        b.style().thumb[2] = style_.scrollThumb[2];
    };
    bar_.setOrientation(Scrollbar::Orient::Vertical);
    bar_.setRect(x_ + w_ - barW, bodyTop, barW - 1, bodyH);
    bar_.setRange(rowsH, bodyH);
    tint(bar_);
    bar_.setValue(scroll_);

    hbar_.setOrientation(Scrollbar::Orient::Horizontal);
    hbar_.setRect(x_ + 1, y_ + h_ - barW, viewW, barW - 1);
    hbar_.setRange(contentW, viewW);
    tint(hbar_);
    hbar_.setValue(hscroll_);
}

void ListView::scrollToSelected(Font& font) {
    if (selected_ < 0) return;
    int rh = rowHeight(font);
    float top = float(selected_ * rh), bottom = top + rh;
    if (top < scroll_)                  scroll_ = top;
    else if (bottom > scroll_ + bodyH_) scroll_ = bottom - bodyH_;
}

bool ListView::update(Window& win, Font& font) {
    activated_ = false;
    int rh = rowHeight(font);
    float hH = headerHeight(font);
    float bodyTop = y_ + hH + 1;
    int prev = selected_;

    layoutBars(font);   // sets viewW_/bodyH_ and both bar geometries

    float mx = win.mouseX(), my = win.mouseY();
    bool inBody = (mx >= x_ && mx < x_ + viewW_ && my >= bodyTop && my < bodyTop + bodyH_);
    bool inWidget = (mx >= x_ && mx < x_ + w_ && my >= y_ && my < y_ + h_);

    // Wheel: vertical, or horizontal when Shift is held.
    if (inBody && win.mouseWheel() != 0) {
        if (win.modShift()) hscroll_ -= win.mouseWheel() * 40.0f;
        else                scroll_  -= win.mouseWheel() * rh * 3.0f;
        bar_.setValue(scroll_);
        hbar_.setValue(hscroll_);
    }

    bool overBar = (bar_.needed() && bar_.hit(mx, my)) || (hbar_.needed() && hbar_.hit(mx, my));

    hover_ = -1;
    if (inBody && !overBar) {
        int row = int((my - bodyTop + scroll_) / rh);
        if (row >= 0 && row < rowCount()) hover_ = row;
    }

    bool inHeader = showHeader_ && (mx >= x_ && mx < x_ + viewW_ && my >= y_ && my < y_ + hH);

    if (win.mousePressed()) {
        focused_ = inWidget;
        if (inHeader) {                          // click a header -> cycle its sort
            int c = columnAtX(mx + hscroll_);    // map screen x to content column
            if (c >= 0 && columns_[c].sortable) {
                if (sortCol_ == c) {
                    if (dir_ == SortDir::Ascending)  dir_ = SortDir::Descending;
                    else { sortCol_ = -1; dir_ = SortDir::None; }
                } else {
                    sortCol_ = c; dir_ = SortDir::Ascending;
                }
                rebuildOrder();
            }
        } else if (!overBar && inBody && hover_ >= 0) {
            selected_ = hover_;
            if (win.mouseClicks() >= 2) activated_ = true;
        }
    }

    bar_.update(win);  scroll_  = bar_.value();
    hbar_.update(win); hscroll_ = hbar_.value();

    if (focused_ && rowCount() > 0) {
        if (win.keyPressed(Key::Up))   selected_ = (selected_ <= 0) ? 0 : selected_ - 1;
        if (win.keyPressed(Key::Down)) selected_ = (selected_ < 0) ? 0 : std::min(selected_ + 1, rowCount() - 1);
        if (win.keyPressed(Key::Home)) selected_ = 0;
        if (win.keyPressed(Key::End))  selected_ = rowCount() - 1;
        int page = std::max(1, int(bodyH_ / rh));
        if (win.keyPressed(Key::PageUp))   selected_ = std::max(0, (selected_ < 0 ? 0 : selected_) - page);
        if (win.keyPressed(Key::PageDown)) selected_ = std::min(rowCount() - 1, (selected_ < 0 ? 0 : selected_) + page);
        if (win.keyPressed(Key::Enter) && selected_ >= 0) activated_ = true;
        if (selected_ != prev) scrollToSelected(font);
    }

    scroll_ = std::clamp(scroll_, 0.0f, maxScroll(font));
    return selected_ != prev;
}

void ListView::draw(SDL_Renderer* renderer, Font& font) {
    layoutBars(font);   // sets viewW_/bodyH_ and bar geometries

    const int rh = rowHeight(font);
    const int lineH = font.lineHeight();
    const float hH = headerHeight(font);
    const float bodyTop = y_ + hH + 1;
    const float cellPad = 8.0f;
    const float viewLeft = x_ + 1;
    const float viewRight = x_ + 1 + viewW_;   // right edge of the content area (before vbar)

    SDL_FRect rect{ x_, y_, w_, h_ };
    SDL_SetRenderDrawColor(renderer, style_.bg[0], style_.bg[1], style_.bg[2], 255);
    SDL_RenderFillRect(renderer, &rect);

    // Draws a cell; colX is already shifted by -hscroll_. Clips to the column
    // intersected with the content viewport horizontally, and with the caller's
    // region [clipTop, clipBot] vertically (the header band or the body band),
    // so a partially-scrolled body row never paints over the header.
    auto drawCell = [&](const std::string& text, const Column& col, float colX,
                        float rowY, float rowH, float clipTop, float clipBot,
                        const unsigned char* color) {
        float cl = std::max(colX, viewLeft);
        float cr = std::min(colX + col.width, viewRight);
        float ct = std::max(rowY, clipTop);
        float cb = std::min(rowY + rowH, clipBot);
        if (cr <= cl || cb <= ct) return;
        SDL_Rect clip{ int(cl), int(ct), int(cr - cl), int(cb - ct) };
        SDL_SetRenderClipRect(renderer, &clip);
        int tw = 0, th = 0;
        font.measure(text, &tw, &th);
        float tx = (col.align == Align::Right) ? colX + col.width - tw - cellPad : colX + cellPad;
        font.draw(text, tx, rowY + (rowH - lineH) * 0.5f, color[0], color[1], color[2]);
    };

    // Header (fixed vertically; scrolls horizontally with the body).
    if (showHeader_) {
        SDL_FRect hdr{ x_, y_, w_, hH };
        SDL_SetRenderDrawColor(renderer, style_.headerBg[0], style_.headerBg[1], style_.headerBg[2], 255);
        SDL_RenderFillRect(renderer, &hdr);
        const float kSortIconW = 16.0f;   // reserve room for the sort arrow
        for (int c = 0; c < int(columns_.size()); ++c) {
            Column hc = columns_[c];
            if (hc.sortable) hc.width = std::max(0.0f, hc.width - kSortIconW);
            drawCell(columns_[c].title, hc, columnX(c) - hscroll_, y_, hH, y_, y_ + hH, style_.headerText);
        }
        SDL_SetRenderClipRect(renderer, nullptr);

        // Sort arrow on the active column: a small filled triangle built from a
        // single symmetric formula so ascending (up) and descending (down) are
        // exact mirrors. Snap the center to whole pixels to keep it crisp.
        if (sortCol_ >= 0 && sortCol_ < int(columns_.size()) && dir_ != SortDir::None) {
            float ax = columnX(sortCol_) + columns_[sortCol_].width - 12 - hscroll_;
            if (ax > viewLeft && ax < viewRight) {
                int   cx = int(ax + 0.5f);
                int   cy = int(y_ + hH * 0.5f + 0.5f);
                const int hw = 4, hh = 3;      // half-width, half-height
                bool  up = (dir_ == SortDir::Ascending);
                SDL_SetRenderDrawColor(renderer, style_.headerText[0], style_.headerText[1], style_.headerText[2], 255);
                for (int dy = -hh; dy <= hh; ++dy) {
                    float f = up ? float(dy + hh) / float(2 * hh)   // 0 at top -> full at bottom (apex up)
                                 : float(hh - dy) / float(2 * hh);  // full at top -> 0 at bottom (apex down)
                    float halfW = hw * f;
                    SDL_RenderLine(renderer, cx - halfW, float(cy + dy), cx + halfW, float(cy + dy));
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, style_.border[0], style_.border[1], style_.border[2], 255);
        SDL_RenderLine(renderer, x_, y_ + hH, x_ + w_, y_ + hH);
    }

    // Body rows (clipped to the content viewport, only the visible range).
    SDL_Rect bodyClip{ int(viewLeft), int(bodyTop), int(viewW_), int(bodyH_) };
    SDL_SetRenderClipRect(renderer, &bodyClip);
    int first = std::max(0, int(scroll_ / rh));
    int last  = std::min(rowCount() - 1, int((scroll_ + bodyH_) / rh));
    for (int i = first; i <= last; ++i) {
        SDL_SetRenderClipRect(renderer, &bodyClip);   // restore after per-cell clips
        float rowY = bodyTop + i * rh - scroll_;
        SDL_FRect row{ viewLeft, rowY, viewW_, float(rh) };
        const unsigned char* txt = style_.item;
        if (i == selected_) {
            SDL_SetRenderDrawColor(renderer, style_.selectedBg[0], style_.selectedBg[1], style_.selectedBg[2], 255);
            SDL_RenderFillRect(renderer, &row);
            txt = style_.selectedText;
        } else if (i == hover_) {
            SDL_SetRenderDrawColor(renderer, style_.hoverBg[0], style_.hoverBg[1], style_.hoverBg[2], 255);
            SDL_RenderFillRect(renderer, &row);
        }
        const Row& data = rows_[order_[i]];   // display order (post-sort)
        for (int c = 0; c < int(columns_.size()); ++c) {
            const std::string& cell = (c < int(data.size())) ? data[c] : std::string();
            drawCell(cell, columns_[c], columnX(c) - hscroll_, rowY, float(rh),
                     bodyTop, bodyTop + bodyH_, txt);
        }
    }
    SDL_SetRenderClipRect(renderer, nullptr);

    // Vertical gridlines between columns, plus the right edge of the last column
    // (so it reads as a real column when the widget is wider than the content).
    SDL_SetRenderDrawColor(renderer, style_.gridline[0], style_.gridline[1], style_.gridline[2], 255);
    for (int c = 1; c <= int(columns_.size()); ++c) {
        float gx = columnX(c) - hscroll_;
        if (gx > viewLeft && gx < viewRight) SDL_RenderLine(renderer, gx, y_ + 1, gx, y_ + h_ - 1);
    }

    SDL_SetRenderDrawColor(renderer, style_.border[0], style_.border[1], style_.border[2], 255);
    SDL_RenderRect(renderer, &rect);

    bar_.draw(renderer);
    hbar_.draw(renderer);
}

} // namespace sdlw
