// Headless widget-behavior tests. Drives widget update() logic through a
// headless Window with injected input (no SDL runtime, no display). These
// cover the interaction bugs that pure-logic tests can't reach — including the
// two ComboBox regressions we fixed.
#include "check.h"
#include "sdlw/window.h"
#include "sdlw/font.h"
#include "sdlw/button.h"
#include "sdlw/textbox.h"
#include "sdlw/combobox.h"

#include <string>

using namespace sdlw;

// A deterministic monospace-ish metrics font: every ASCII glyph is 10px wide,
// lineHeight 20, base 16. Lets tests reason about layout without a real atlas.
static std::string makeMonoFnt() {
    std::string s;
    s += "info face=\"mono\" size=16\n";
    s += "common lineHeight=20 base=16 scaleW=256 scaleH=256 pages=1\n";
    s += "page id=0 file=\"mono.bmp\"\n";
    s += "chars count=95\n";
    for (int cp = 32; cp <= 126; ++cp) {
        s += "char id=" + std::to_string(cp) +
             " x=0 y=0 width=8 height=12 xoffset=0 yoffset=4 xadvance=10 page=0 chnl=15\n";
    }
    return s;
}

static void loadMono(Font& f) {
    static std::string fnt = makeMonoFnt();
    f.loadMetrics(reinterpret_cast<const unsigned char*>(fnt.data()),
                  static_cast<unsigned int>(fnt.size()));
}

TEST(button_click_press_then_release_inside) {
    Window win{Window::Headless{}};
    Font font; loadMono(font);
    Button btn("Go", 10, 10, 100, 40); // spans x 10..110, y 10..50

    // Press inside (no click yet on press).
    win.clearFrameInput();
    win.feedMouse(50, 30, true);
    win.feedMousePress(1);
    CHECK(!btn.update(win));
    CHECK(btn.pressed());

    // Release inside -> click fires once.
    win.clearFrameInput();
    win.feedMouse(50, 30, false);
    CHECK(btn.update(win));

    // Press inside then release OUTSIDE -> no click.
    win.clearFrameInput();
    win.feedMouse(50, 30, true); win.feedMousePress(1);
    btn.update(win);
    win.clearFrameInput();
    win.feedMouse(500, 500, false);
    CHECK(!btn.update(win));
}

TEST(textbox_focus_type_and_backspace) {
    Window win{Window::Headless{}};
    Font font; loadMono(font);
    TextBox tb(0, 0, 200, 30);

    // Click to focus.
    win.clearFrameInput();
    win.feedMouse(20, 15, true); win.feedMousePress(1);
    tb.update(win, font);
    CHECK(tb.focused());

    // Type "hi".
    win.clearFrameInput();
    win.feedMouse(20, 15, false); win.feedText("hi");
    tb.update(win, font);
    CHECK_STR_EQ(tb.text(), "hi");

    // Backspace removes one char.
    win.clearFrameInput();
    win.feedKey(Key::Backspace);
    tb.update(win, font);
    CHECK_STR_EQ(tb.text(), "h");
}

// Drives the exact sequence from the two combo bug reports.
TEST(combo_click_commits_and_reopen_browses_all) {
    Window win{Window::Headless{}};
    Font font; loadMono(font);
    ComboBox combo(0, 0, 120, 20);   // field 0..100, arrow 100..120; popup below y=22
    combo.setItems({ "Poland", "Portugal", "Spain", "Sweden", "France" });

    // Click the field to focus + open.
    win.clearFrameInput();
    win.feedMouse(40, 10, true); win.feedMousePress(1);
    combo.update(win, font);

    // Type "po" over two frames.
    win.clearFrameInput(); win.feedMouse(40, 10, false); win.feedText("p"); combo.update(win, font);
    win.clearFrameInput(); win.feedMouse(40, 10, false); win.feedText("o"); combo.update(win, font);
    CHECK(combo.isOpen());
    CHECK_EQ(combo.list().count(), 2); // Poland, Portugal

    // Click row 0 (Poland). rowHeight = lineHeight(20)+6 = 26; popup top = y+h+2 = 22.
    // Row 0 spans y 23..49 -> click at y=30. (Bug #1: was ignored because row 0
    // was already the highlighted selection.)
    win.clearFrameInput();
    win.feedMouse(30, 30, true); win.feedMousePress(1);
    bool committed = combo.update(win, font);
    CHECK(committed);
    CHECK_STR_EQ(combo.text(), "Poland");
    CHECK(!combo.isOpen());

    // Reopen via the arrow (x 100..120). (Bug #2: used to show only the stale
    // 2-item filter; must now browse the full list.)
    win.clearFrameInput();
    win.feedMouse(110, 10, true); win.feedMousePress(1);
    combo.update(win, font);
    CHECK(combo.isOpen());
    CHECK_EQ(combo.list().count(), 5); // full list again
}

SDLW_TEST_MAIN()
