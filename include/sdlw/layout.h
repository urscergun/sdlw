// sdlw layout - compute widget rectangles from a declarative tree.
//
// A layout is a tree of Box (row/column) and Grid nodes. Each node's job is to
// take a rectangle and assign sub-rectangles to its children by calling their
// setRect(x, y, w, h). Every sdlw widget already has that method, and Box/Grid
// provide it too (delegating to arrange), so nodes nest freely.
//
// Sizing along a box's main axis is per-child: Fixed(px) or Flex(weight). Flex
// children share the leftover space by weight (after fixed sizes + flex minima).
// On the cross axis children stretch to fill; use spacers to center/distribute.
//
// This header is pure C++ (no SDL) so arrange() is trivially unit-testable.
//
// Example:
//     sdlw::VBox form({.padding = 12, .spacing = 8});
//     form.add(nameLabel, sdlw::Size::fixed(20));
//     form.add(nameBox,   sdlw::Size::fixed(30));
//     form.add(list,      sdlw::Size::flex());     // fills remaining height
//     form.arrange({0, 0, float(win.width()), float(win.height())});
#pragma once

#include <algorithm>
#include <functional>
#include <vector>

namespace sdlw {

struct Rect { float x = 0, y = 0, w = 0, h = 0; };

// Per-child sizing along a box's main axis (or a grid track's size).
struct Size {
    enum Kind { Fixed, Flex };
    Kind  kind = Fixed;
    float value = 0;   // pixels for Fixed, weight for Flex
    float min = 0;     // minimum pixels (both kinds)
    static Size fixed(float px)                  { return { Fixed, px, 0 }; }
    static Size flex(float weight = 1, float minPx = 0) { return { Flex, weight, minPx }; }
};

// Container padding (inset on all sides) and spacing (gap between children).
struct BoxOpts { float padding = 0; float spacing = 0; };

class Box {
public:
    enum class Dir { Row, Column };
    using Opts = BoxOpts;

    Box(Dir dir, BoxOpts opts = BoxOpts{}) : dir_(dir), opts_(opts) {}

    // Add anything with setRect(float,float,float,float): a widget or a nested
    // Box/Grid. `main` is its size along this box's main axis.
    template <class T>
    Box& add(T& node, Size main) {
        children_.push_back({ [&node](Rect r) { node.setRect(r.x, r.y, r.w, r.h); }, main });
        return *this;
    }
    // Empty space (defaults to flexible).
    Box& addSpacer(Size main = Size::flex()) {
        children_.push_back({ nullptr, main });
        return *this;
    }

    void arrange(Rect r);
    void setRect(float x, float y, float w, float h) { arrange({ x, y, w, h }); }
    void clear() { children_.clear(); }

private:
    struct Child { std::function<void(Rect)> place; Size main; };
    Dir     dir_;
    BoxOpts opts_;
    std::vector<Child> children_;
};

// Convenience: fixed-direction boxes.
struct VBox : Box { explicit VBox(BoxOpts o = BoxOpts{}) : Box(Dir::Column, o) {} };
struct HBox : Box { explicit HBox(BoxOpts o = BoxOpts{}) : Box(Dir::Row, o) {} };

// A grid with Fixed/Flex column and row tracks. Cells are placed by (col, row)
// and may span multiple tracks.
struct GridOpts { float padding = 0; float colSpacing = 0; float rowSpacing = 0; };

class Grid {
public:
    Grid(std::vector<Size> cols, std::vector<Size> rows, GridOpts opts = GridOpts{})
        : cols_(std::move(cols)), rows_(std::move(rows)), opts_(opts) {}

    template <class T>
    Grid& add(T& node, int col, int row, int colSpan = 1, int rowSpan = 1) {
        cells_.push_back({ [&node](Rect r) { node.setRect(r.x, r.y, r.w, r.h); },
                           col, row, colSpan, rowSpan });
        return *this;
    }
    void setColumns(std::vector<Size> cols) { cols_ = std::move(cols); }
    void setRows(std::vector<Size> rows)    { rows_ = std::move(rows); }
    void clear() { cells_.clear(); }

    void arrange(Rect r);
    void setRect(float x, float y, float w, float h) { arrange({ x, y, w, h }); }

private:
    struct Cell { std::function<void(Rect)> place; int col, row, cspan, rspan; };
    std::vector<Size> cols_, rows_;
    GridOpts opts_;
    std::vector<Cell> cells_;
};

} // namespace sdlw
