// Example sdlw application: a gallery showcasing every control.
//
// Left column:  text field, editable combo box, non-editable select, checkbox,
//               radio group. Right column: scrollable list, progress bar, and
//               buttons. Tab / Shift+Tab move keyboard focus (ring); Space/Enter
//               activate; arrow keys drive the radio and list. All fonts are
//               embedded — no external files. There is no WinMain/main here;
//               sdlw provides the entry point and calls Main().
#include "sdlw/window.h"
#include "sdlw/font.h"
#include "sdlw/label.h"
#include "sdlw/textbox.h"
#include "sdlw/combobox.h"
#include "sdlw/select.h"
#include "sdlw/checkbox.h"
#include "sdlw/radiogroup.h"
#include "sdlw/listbox.h"
#include "sdlw/progressbar.h"
#include "sdlw/button.h"
#include "sdlw/focus.h"

#include <cstdio>
#include <string>

#define SDLW_DECL_FONT(sz)                                     \
    extern "C" {                                               \
        extern const unsigned char dejavusans_##sz##_fnt[];    \
        extern const unsigned int  dejavusans_##sz##_fnt_len;  \
        extern const unsigned char dejavusans_##sz##_bmp[];    \
        extern const unsigned int  dejavusans_##sz##_bmp_len;  \
    }
SDLW_DECL_FONT(16)
SDLW_DECL_FONT(24)

int Main(int argc, char** argv) {
    (void)argc; (void)argv;

    sdlw::Window win({ .title = "sdlw — control gallery", .width = 720, .height = 520 });
    if (!win.ok()) { std::fprintf(stderr, "window: %s\n", win.error()); return 1; }

    sdlw::Font heading, ui;
    if (!heading.loadFromMemory(win.renderer(), dejavusans_24_fnt, dejavusans_24_fnt_len,
                                dejavusans_24_bmp, dejavusans_24_bmp_len) ||
        !ui.loadFromMemory(win.renderer(), dejavusans_16_fnt, dejavusans_16_fnt_len,
                           dejavusans_16_bmp, dejavusans_16_bmp_len)) {
        std::fprintf(stderr, "font: %s\n", ui.error());
        return 1;
    }

    sdlw::Label title("sdlw — control gallery", 20, 16);

    // Left column ----------------------------------------------------------
    sdlw::Label   nameL("Name", 20, 52);
    sdlw::TextBox name(20, 72, 300, 30);
    name.setPlaceholder("Type your name...");

    sdlw::Label    countryL("Country (combo — type to filter)", 20, 114);
    sdlw::ComboBox country(20, 134, 300, 30);
    country.setMaxVisibleRows(4);
    country.setItems({ "Argentina", "Australia", "Brazil", "Canada", "China", "Denmark",
                       "Egypt", "France", "Germany", "India", "Italy", "Japan", "Mexico",
                       "Norway", "Poland", "Spain", "Sweden", "Switzerland" });

    sdlw::Label   langL("Language (select)", 20, 286);
    sdlw::Select  lang(20, 306, 300, 30);
    lang.setMaxVisibleRows(4);
    lang.setPlaceholder("Choose...");
    lang.setItems({ "C", "C++", "Python", "Rust", "Go", "Zig", "Lua" });

    sdlw::Checkbox subscribe("Email me updates", 20, 358, 300, 24);

    sdlw::Label      planL("Plan", 20, 392);
    sdlw::RadioGroup plan(20, 412, 200, { "Free", "Pro", "Team" });

    // Right column ---------------------------------------------------------
    sdlw::Label   itemsL("Items (list)", 380, 52);
    sdlw::ListBox items(380, 72, 320, 200);
    items.setItems({ "Alpha", "Bravo", "Charlie", "Delta", "Echo", "Foxtrot", "Golf",
                     "Hotel", "India", "Juliet", "Kilo", "Lima", "Mike", "November" });
    items.setSelected(0);

    sdlw::Label       progL("Progress", 380, 292);
    sdlw::ProgressBar bar(380, 312, 320, 18);
    bar.setShowPercent(true);

    sdlw::Button submit("Submit", 380, 350, 150, 34);
    sdlw::Button reset("Reset",  550, 350, 150, 34);

    sdlw::Label status("", 380, 410);
    status.style().color[0] = 150; status.style().color[1] = 230; status.style().color[2] = 170;

    // Tab order (the composites drive their own focus via the mouse).
    sdlw::FocusManager focus;
    focus.add(&name);
    focus.add(&subscribe);
    focus.add(&plan);
    focus.add(&items);
    focus.add(&submit);
    focus.add(&reset);

    float t = 0;
    while (win.pumpEvents()) {
        focus.update(win);
        name.update(win, ui);
        country.update(win, ui);
        lang.update(win, ui);
        subscribe.update(win);
        plan.update(win);
        items.update(win, ui);
        if (submit.update(win)) {
            std::string who = name.text().empty() ? "(no name)" : name.text();
            std::string ctry = country.text().empty() ? "-" : country.text();
            status.setText("Submitted: " + who + " / " + ctry);
        }
        if (reset.update(win)) {
            name.setText(""); country.setText(""); lang.setSelected(-1);
            subscribe.setChecked(false); plan.setSelected(0); status.setText("");
        }

        t += 0.004f; if (t > 1.0f) t = 0.0f;
        bar.setValue(t);

        win.clear(24, 24, 32);
        title.style().color[0] = 120; title.style().color[1] = 200; title.style().color[2] = 255;
        title.draw(win.renderer(), heading);

        nameL.draw(win.renderer(), ui);
        name.draw(win.renderer(), ui);
        countryL.draw(win.renderer(), ui);
        langL.draw(win.renderer(), ui);
        subscribe.draw(win.renderer(), ui);
        planL.draw(win.renderer(), ui);
        plan.draw(win.renderer(), ui);

        itemsL.draw(win.renderer(), ui);
        items.draw(win.renderer(), ui);
        progL.draw(win.renderer(), ui);
        bar.draw(win.renderer(), ui);
        submit.draw(win.renderer(), ui);
        reset.draw(win.renderer(), ui);
        status.draw(win.renderer(), ui);

        // Dropdowns last so their popups render on top of everything.
        country.draw(win.renderer(), ui);
        lang.draw(win.renderer(), ui);

        focus.drawFocusRing(win.renderer());
        win.present();
    }
    return 0;
}
