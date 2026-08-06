// Unit tests for the layout engine (src/layout.cpp) — pure rectangle math,
// no SDL. A tiny stand-in widget records the rect it was assigned.
#include "check.h"
#include "sdlw/layout.h"

using namespace sdlw;

struct W {
    Rect r;
    void setRect(float x, float y, float w, float h) { r = { x, y, w, h }; }
};

TEST(vbox_fixed_and_flex_fills_remaining) {
    W a, b, c;
    VBox v;
    v.add(a, Size::fixed(20));
    v.add(b, Size::flex());       // takes the leftover height
    v.add(c, Size::fixed(30));
    v.arrange({ 0, 0, 100, 200 });

    CHECK_EQ(a.r.y, 0);   CHECK_EQ(a.r.h, 20);
    CHECK_EQ(b.r.y, 20);  CHECK_EQ(b.r.h, 150);   // 200 - 20 - 30
    CHECK_EQ(c.r.y, 170); CHECK_EQ(c.r.h, 30);
    CHECK_EQ(a.r.x, 0);   CHECK_EQ(a.r.w, 100);   // cross axis stretches
}

TEST(hbox_flex_by_weight) {
    W a, b;
    HBox h;
    h.add(a, Size::flex(1));
    h.add(b, Size::flex(2));
    h.arrange({ 0, 0, 300, 50 });
    CHECK_EQ(a.r.x, 0);   CHECK_EQ(a.r.w, 100);
    CHECK_EQ(b.r.x, 100); CHECK_EQ(b.r.w, 200);
    CHECK_EQ(a.r.h, 50);
}

TEST(padding_and_spacing) {
    W a, b;
    VBox v({ .padding = 10, .spacing = 8 });
    v.add(a, Size::flex());
    v.add(b, Size::flex());
    v.arrange({ 0, 0, 100, 100 });
    // inner height 80, minus 8 spacing = 72 shared -> 36 each.
    CHECK_EQ(a.r.y, 10); CHECK_EQ(a.r.h, 36);
    CHECK_EQ(b.r.y, 54); CHECK_EQ(b.r.h, 36);     // 10 + 36 + 8
    CHECK_EQ(a.r.x, 10); CHECK_EQ(a.r.w, 80);
}

TEST(spacers_center_a_fixed_child) {
    W btn;
    HBox h;
    h.addSpacer(Size::flex(1));
    h.add(btn, Size::fixed(120));
    h.addSpacer(Size::flex(1));
    h.arrange({ 0, 0, 400, 40 });
    CHECK_EQ(btn.r.x, 140); CHECK_EQ(btn.r.w, 120);   // (400-120)/2 = 140
}

TEST(flex_minimum_is_respected) {
    W a, b;
    HBox h;
    h.add(a, Size::flex(1, /*min*/ 80));
    h.add(b, Size::flex(1));
    h.arrange({ 0, 0, 100, 20 });
    // minima (80) guaranteed first; leftover 20 split 1:1 -> a=90, b=10.
    CHECK_EQ(a.r.w, 90); CHECK_EQ(b.r.w, 10);
}

TEST(nested_boxes) {
    W a, b, c;
    VBox col;
    col.add(a, Size::flex());
    col.add(b, Size::flex());
    HBox row;
    row.add(col, Size::flex(1));    // a nested box as a child
    row.add(c, Size::fixed(100));
    row.arrange({ 0, 0, 300, 200 });

    CHECK_EQ(c.r.x, 200); CHECK_EQ(c.r.w, 100);       // fixed column on the right
    CHECK_EQ(a.r.w, 200); CHECK_EQ(a.r.h, 100);       // col filled the rest, split vertically
    CHECK_EQ(b.r.y, 100); CHECK_EQ(b.r.h, 100);
}

TEST(grid_tracks_and_spanning) {
    W a, b, c, d;
    // 2 columns (100 fixed, flex), 2 rows (30 fixed, flex), no spacing/padding.
    Grid g({ Size::fixed(100), Size::flex() }, { Size::fixed(30), Size::flex() });
    g.add(a, /*col*/0, /*row*/0);
    g.add(b, 1, 0);
    g.add(c, 0, 1);
    g.add(d, 1, 1);
    g.arrange({ 0, 0, 300, 200 });

    // Column 0 = 100 wide, column 1 = 200. Row 0 = 30 tall, row 1 = 170.
    CHECK_EQ(a.r.x, 0);   CHECK_EQ(a.r.w, 100); CHECK_EQ(a.r.y, 0);  CHECK_EQ(a.r.h, 30);
    CHECK_EQ(b.r.x, 100); CHECK_EQ(b.r.w, 200); CHECK_EQ(b.r.h, 30);
    CHECK_EQ(c.r.y, 30);  CHECK_EQ(c.r.h, 170); CHECK_EQ(c.r.w, 100);
    CHECK_EQ(d.r.x, 100); CHECK_EQ(d.r.y, 30);  CHECK_EQ(d.r.w, 200); CHECK_EQ(d.r.h, 170);
}

TEST(grid_col_span_and_spacing) {
    W header, cell;
    Grid g({ Size::flex(), Size::flex() }, { Size::fixed(20), Size::fixed(20) },
           { .padding = 0, .colSpacing = 10, .rowSpacing = 5 });
    g.add(header, 0, 0, /*colSpan*/2);   // spans both columns
    g.add(cell,   1, 1);
    g.arrange({ 0, 0, 210, 45 });

    // Two flex cols in 210 with 10 gap -> (210-10)/2 = 100 each; col1 at x=110.
    CHECK_EQ(header.r.x, 0);   CHECK_EQ(header.r.w, 210); CHECK_EQ(header.r.h, 20); // spans full width incl gap
    CHECK_EQ(cell.r.x, 110);   CHECK_EQ(cell.r.w, 100);
    CHECK_EQ(cell.r.y, 25);    CHECK_EQ(cell.r.h, 20);    // row1 at 20 + 5 spacing
}

SDLW_TEST_MAIN()
