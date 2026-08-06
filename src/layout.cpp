#include "sdlw/layout.h"

#include <cmath>

namespace sdlw {

namespace {
// Distribute `avail` across tracks (Fixed take their size, Flex share the rest
// by weight after minima). Fills sizes[] and offsets[] (offset includes spacing).
void distributeTracks(const std::vector<Size>& tracks, float avail, float spacing,
                      std::vector<float>& sizes, std::vector<float>& offsets) {
    int n = int(tracks.size());
    sizes.assign(n, 0);
    offsets.assign(n, 0);
    if (n == 0) return;

    float mainAvail = std::max(0.0f, avail - spacing * (n - 1));
    float fixedTotal = 0, weight = 0;
    for (const Size& t : tracks) {
        if (t.kind == Size::Fixed) fixedTotal += std::max(t.value, t.min);
        else { fixedTotal += t.min; weight += std::max(0.0f, t.value); }
    }
    float leftover = std::max(0.0f, mainAvail - fixedTotal);

    float off = 0;
    for (int i = 0; i < n; ++i) {
        const Size& t = tracks[i];
        sizes[i] = (t.kind == Size::Fixed)
                       ? std::max(t.value, t.min)
                       : t.min + (weight > 0 ? leftover * (std::max(0.0f, t.value) / weight) : 0.0f);
        offsets[i] = off;
        off += sizes[i] + spacing;
    }
}
} // namespace

void Box::arrange(Rect r) {
    int n = int(children_.size());
    if (n == 0) return;

    const float pad = opts_.padding, sp = opts_.spacing;
    const float ix = r.x + pad, iy = r.y + pad;
    const float iw = std::max(0.0f, r.w - 2 * pad);
    const float ih = std::max(0.0f, r.h - 2 * pad);
    const bool  row = (dir_ == Dir::Row);

    const float mainStart   = row ? ix : iy;
    const float crossStart  = row ? iy : ix;
    const float crossExtent = row ? ih : iw;
    float mainAvail = (row ? iw : ih) - sp * (n - 1);
    if (mainAvail < 0) mainAvail = 0;

    // Fixed sizes + flex minima are guaranteed; leftover is shared by weight.
    float fixedTotal = 0, weight = 0;
    for (const Child& c : children_) {
        if (c.main.kind == Size::Fixed) fixedTotal += std::max(c.main.value, c.main.min);
        else { fixedTotal += c.main.min; weight += std::max(0.0f, c.main.value); }
    }
    const float leftover = std::max(0.0f, mainAvail - fixedTotal);

    // Place children, rounding edges so neighbours share a boundary (no gaps).
    float pos = mainStart;
    for (int i = 0; i < n; ++i) {
        const Size& s = children_[i].main;
        float size = (s.kind == Size::Fixed)
                         ? std::max(s.value, s.min)
                         : s.min + (weight > 0 ? leftover * (std::max(0.0f, s.value) / weight) : 0.0f);

        float a = std::round(pos), b = std::round(pos + size);
        Rect cr = row ? Rect{ a, std::round(crossStart), b - a, std::round(crossExtent) }
                      : Rect{ std::round(crossStart), a, std::round(crossExtent), b - a };
        if (children_[i].place) children_[i].place(cr);
        pos += size + sp;
    }
}

void Grid::arrange(Rect r) {
    const float pad = opts_.padding;
    const float ix = r.x + pad, iy = r.y + pad;
    const float iw = std::max(0.0f, r.w - 2 * pad);
    const float ih = std::max(0.0f, r.h - 2 * pad);

    std::vector<float> colSize, colOff, rowSize, rowOff;
    distributeTracks(cols_, iw, opts_.colSpacing, colSize, colOff);
    distributeTracks(rows_, ih, opts_.rowSpacing, rowSize, rowOff);

    int nc = int(cols_.size()), nr = int(rows_.size());
    for (const Cell& c : cells_) {
        if (c.col < 0 || c.row < 0 || c.col >= nc || c.row >= nr) continue;
        int lastC = std::min(nc - 1, c.col + std::max(1, c.cspan) - 1);
        int lastR = std::min(nr - 1, c.row + std::max(1, c.rspan) - 1);

        float left   = ix + colOff[c.col];
        float right  = ix + colOff[lastC] + colSize[lastC];
        float top    = iy + rowOff[c.row];
        float bottom = iy + rowOff[lastR] + rowSize[lastR];

        float l = std::round(left), t = std::round(top);
        Rect cr{ l, t, std::round(right) - l, std::round(bottom) - t };
        if (c.place) c.place(cr);
    }
}

} // namespace sdlw
