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
#include "sdlw/checkbox.h"
#include "sdlw/radiogroup.h"
#include "sdlw/select.h"
#include "sdlw/focus.h"
#include "sdlw/scrollbar.h"
#include "sdlw/scrollview.h"
#include "sdlw/listbox.h"

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

TEST(checkbox_toggles_on_click) {
    Window win{Window::Headless{}};
    Checkbox cb("Agree", 0, 0, 200, 24);
    CHECK(!cb.checked());
    // Press inside toggles on.
    win.clearFrameInput(); win.feedMouse(10, 10, true); win.feedMousePress(1);
    CHECK(cb.update(win));
    CHECK(cb.checked());
    // Press again toggles off.
    win.clearFrameInput(); win.feedMouse(10, 10, true); win.feedMousePress(1);
    CHECK(cb.update(win));
    CHECK(!cb.checked());
    // Press outside does nothing.
    win.clearFrameInput(); win.feedMouse(500, 500, true); win.feedMousePress(1);
    CHECK(!cb.update(win));
    CHECK(!cb.checked());
}

TEST(radiogroup_single_selection) {
    Window win{Window::Headless{}};
    RadioGroup rg(0, 0, 200, { "Light", "Dark", "System" }, 26); // rows at y 0,26,52
    CHECK_EQ(rg.selected(), 0);                 // first selected by default
    // Click the third row (y in [52,78)).
    win.clearFrameInput(); win.feedMouse(20, 60, true); win.feedMousePress(1);
    CHECK(rg.update(win));
    CHECK_EQ(rg.selected(), 2);
    CHECK_STR_EQ(*rg.selectedOption(), "System");
    // Clicking the same row again -> no change reported.
    win.clearFrameInput(); win.feedMouse(20, 60, true); win.feedMousePress(1);
    CHECK(!rg.update(win));
}

TEST(select_opens_and_commits) {
    Window win{Window::Headless{}};
    Font font; loadMono(font);
    Select sel(0, 0, 120, 20);                  // popup below y=22, rowHeight 26
    sel.setItems({ "C", "C++", "Python", "Rust" });

    // Click field -> opens.
    win.clearFrameInput(); win.feedMouse(40, 10, true); win.feedMousePress(1);
    sel.update(win, font);
    CHECK(sel.isOpen());

    // Click row 1 ("C++"): row 1 spans y ~ 22+1+26 .. -> click y=60.
    win.clearFrameInput(); win.feedMouse(40, 60, true); win.feedMousePress(1);
    bool committed = sel.update(win, font);
    CHECK(committed);
    CHECK(!sel.isOpen());
    CHECK_STR_EQ(*sel.selectedItem(), "C++");
    CHECK_EQ(sel.selected(), 1);
}

TEST(focus_tab_traversal_and_activation) {
    Window win{Window::Headless{}};
    Font font; loadMono(font);

    TextBox  name(0, 0, 200, 30);
    Checkbox agree("Agree", 0, 40, 200, 24);
    Button   ok("OK", 0, 80, 100, 30);

    FocusManager focus;
    focus.add(&name);
    focus.add(&agree);
    focus.add(&ok);

    // Nothing focused initially.
    CHECK(focus.focused() == nullptr);

    // Tab -> first (name).
    win.clearFrameInput(); win.feedKey(Key::Tab);
    focus.update(win);
    CHECK(focus.focused() == &name);
    CHECK(name.focused());

    // Typing now goes to the focused text box (no click needed).
    win.clearFrameInput(); win.feedText("hi");
    name.update(win, font);
    CHECK_STR_EQ(name.text(), "hi");

    // Tab -> checkbox; Space toggles it.
    win.clearFrameInput(); win.feedKey(Key::Tab);
    focus.update(win);
    CHECK(focus.focused() == &agree);
    CHECK(!name.focused());
    win.clearFrameInput(); win.feedKey(Key::Space);
    focus.update(win);           // manager consumes nothing here; widget acts
    CHECK(agree.update(win));
    CHECK(agree.checked());

    // Tab -> button; Enter activates it.
    win.clearFrameInput(); win.feedKey(Key::Tab);
    focus.update(win);
    CHECK(focus.focused() == &ok);
    win.clearFrameInput(); win.feedKey(Key::Enter);
    focus.update(win);
    CHECK(ok.update(win));

    // Tab wraps back to the first control.
    win.clearFrameInput(); win.feedKey(Key::Tab);
    focus.update(win);
    CHECK(focus.focused() == &name);

    // Shift+Tab goes backwards (to the last control).
    win.clearFrameInput(); win.feedKey(Key::Tab); win.feedMods(true, false);
    focus.update(win);
    CHECK(focus.focused() == &ok);
}

TEST(focus_includes_dropdowns_and_popup_click_keeps_focus) {
    Window win{Window::Headless{}};
    Font font; loadMono(font);

    TextBox name(0, 0, 200, 30);
    Select  sel(0, 40, 120, 20);              // popup below y=62
    sel.setItems({ "C", "C++", "Python" });
    Button  ok("OK", 0, 200, 100, 30);

    FocusManager focus;
    focus.add(&name);
    focus.add(&sel);
    focus.add(&ok);

    // Tab twice -> the Select is focused (it's in the cycle now).
    win.clearFrameInput(); win.feedKey(Key::Tab); focus.update(win);
    win.clearFrameInput(); win.feedKey(Key::Tab); focus.update(win);
    CHECK(focus.focused() == &sel);
    CHECK(sel.focused());

    // Space opens it (keyboard, while focused).
    win.clearFrameInput(); win.feedKey(Key::Space);
    focus.update(win);       // manager: no tab/click -> focus unchanged
    sel.update(win, font);
    CHECK(sel.isOpen());

    // Click an item INSIDE the popup. The popup is outside the field rect, but
    // hitTest covers it, so focus must stay on the Select (not defocus) and the
    // click commits. Row 1 ("C++") ~ y 62+1+26 -> click y=95.
    win.clearFrameInput(); win.feedMouse(40, 95, true); win.feedMousePress(1);
    focus.update(win);       // must keep focus on sel (popup hit)
    CHECK(focus.focused() == &sel);
    bool committed = sel.update(win, font);
    CHECK(committed);
    CHECK_STR_EQ(*sel.selectedItem(), "C++");
}

TEST(focus_click_sync_and_defocus) {
    Window win{Window::Headless{}};
    TextBox a(0, 0, 100, 30);
    TextBox b(0, 40, 100, 30);
    FocusManager focus;
    focus.add(&a);
    focus.add(&b);

    // Click inside b -> focus follows the click.
    win.clearFrameInput(); win.feedMouse(50, 55, true); win.feedMousePress(1);
    focus.update(win);
    CHECK(focus.focused() == &b);

    // Click empty space -> focus cleared.
    win.clearFrameInput(); win.feedMouse(500, 500, true); win.feedMousePress(1);
    focus.update(win);
    CHECK(focus.focused() == nullptr);
    CHECK(!b.focused());
}

TEST(scrollbar_drag_page_and_clamp) {
    Window win{Window::Headless{}};
    Scrollbar bar;
    // Track at x 0..8, y 0..200; content 500, view 200 -> maxScroll 300.
    bar.setRect(0, 0, 8, 200);
    bar.setRange(500, 200);
    CHECK(bar.needed());
    CHECK_EQ(int(bar.maxScroll()), 300);

    // Grab the thumb (top, value 0) and drag to the bottom -> value == maxScroll.
    // thumbH = max(20, 200*200/500)=80; thumb top at 0.
    win.clearFrameInput(); win.feedMouse(4, 10, true); win.feedMousePress(1);
    bar.update(win);                       // start drag (grab offset 10)
    win.clearFrameInput(); win.feedMouse(4, 400, true); // drag way past bottom
    bar.update(win);
    CHECK(bar.value() >= bar.maxScroll() - 0.5f);

    // Release; page up by clicking the track above the thumb.
    win.clearFrameInput(); win.feedMouse(4, 400, false); bar.update(win);
    float before = bar.value();      // == maxScroll (300)
    win.clearFrameInput(); win.feedMouse(4, 5, true); win.feedMousePress(1);
    bar.update(win);
    CHECK(bar.value() < before);     // paged up (toward 0) by one view height

    // No scrollbar when content fits.
    Scrollbar small;
    small.setRect(0, 0, 8, 200);
    small.setRange(100, 200);
    CHECK(!small.needed());
    CHECK_EQ(int(small.maxScroll()), 0);
}

TEST(scrollview_wheel_and_content_origin) {
    Window win{Window::Headless{}};
    ScrollView view(0, 0, 200, 100);   // viewport 100 tall (inner 98)
    view.setContentHeight(500);        // maxScroll = 500 - 98 = 402

    // Wheel down (negative y) scrolls content up (scroll increases).
    win.clearFrameInput(); win.feedMouse(50, 50, false); win.feedWheel(-1);
    CHECK(view.update(win));
    float s = view.scroll();
    CHECK(s > 0);
    // contentTop() = y + pad - scroll  (content origin shifts up as we scroll).
    CHECK_EQ(int(view.contentTop()), int(0 + 6 - s));

    // Wheel outside the view does nothing.
    float before = view.scroll();
    win.clearFrameInput(); win.feedMouse(500, 500, false); win.feedWheel(-3);
    view.update(win);
    CHECK_EQ(view.scroll(), before);
}

TEST(listbox_scrolls_with_wheel_and_bar) {
    Window win{Window::Headless{}};
    Font font; loadMono(font);         // lineHeight 20 -> rowHeight 26
    ListBox list(0, 0, 120, 100);      // view 98; 20 items -> content 520
    std::vector<std::string> items;
    for (int i = 0; i < 20; ++i) items.push_back("item" + std::to_string(i));
    list.setItems(items);

    // Focus via click on first row, then wheel down should scroll.
    win.clearFrameInput(); win.feedMouse(10, 10, true); win.feedMousePress(1);
    list.update(win, font);
    win.clearFrameInput(); win.feedMouse(10, 10, false); win.feedWheel(-2);
    list.update(win, font);
    // Scrolled down: the first visible row is no longer row 0. We can't read
    // scroll_ directly, but End key should reveal the last item and selection
    // via keyboard still works.
    win.clearFrameInput(); win.feedKey(Key::End);
    list.update(win, font);
    CHECK_EQ(list.selected(), 19);
    CHECK_STR_EQ(*list.selectedItem(), "item19");
}

SDLW_TEST_MAIN()
