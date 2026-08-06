// Example sdlw application: a large window exercising every widget.
//
// Column 1 (form):  text field, combo box, select, checkbox, radio group,
//                   progress bar, buttons.
// Column 2 (lists): a scrollable ListBox and a ScrollView panel of content.
// Column 3 (table): a multi-column ListView.
//
// Tab / Shift+Tab move keyboard focus (ring); Space/Enter activate; arrow keys
// drive the radio, list, and table. All fonts are embedded — no external files.
// There is no WinMain/main here; sdlw supplies the entry point and calls Main().
#include "sdlw/window.h"
#include "sdlw/font.h"
#include "sdlw/label.h"
#include "sdlw/textbox.h"
#include "sdlw/combobox.h"
#include "sdlw/select.h"
#include "sdlw/checkbox.h"
#include "sdlw/radiogroup.h"
#include "sdlw/listbox.h"
#include "sdlw/listview.h"
#include "sdlw/progressbar.h"
#include "sdlw/button.h"
#include "sdlw/scrollview.h"
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

    sdlw::Window win({ .title = "sdlw — all widgets", .width = 1040, .height = 700 });
    if (!win.ok()) { std::fprintf(stderr, "window: %s\n", win.error()); return 1; }

    sdlw::Font heading, ui;
    if (!heading.loadFromMemory(win.renderer(), dejavusans_24_fnt, dejavusans_24_fnt_len,
                                dejavusans_24_bmp, dejavusans_24_bmp_len) ||
        !ui.loadFromMemory(win.renderer(), dejavusans_16_fnt, dejavusans_16_fnt_len,
                           dejavusans_16_bmp, dejavusans_16_bmp_len)) {
        std::fprintf(stderr, "font: %s\n", ui.error());
        return 1;
    }

    sdlw::Label title("sdlw — all widgets", 20, 16);

    // --- Column 1: form --------------------------------------------------
    sdlw::Label   nameL("Name", 20, 60);
    sdlw::TextBox name(20, 80, 300, 30);
    name.setPlaceholder("Type your name...");

    sdlw::Label    countryL("Country (combo — type to filter)", 20, 122);
    sdlw::ComboBox country(20, 142, 300, 30);
    country.setMaxVisibleRows(4);
    country.setItems({ "Argentina", "Australia", "Brazil", "Canada", "China", "Denmark",
                       "Egypt", "France", "Germany", "India", "Italy", "Japan", "Mexico",
                       "Norway", "Poland", "Spain", "Sweden", "Switzerland" });

    sdlw::Label   langL("Language (select)", 20, 300);
    sdlw::Select  lang(20, 320, 300, 30);
    lang.setMaxVisibleRows(4);
    lang.setPlaceholder("Choose...");
    lang.setItems({ "C", "C++", "Python", "Rust", "Go", "Zig", "Lua" });

    sdlw::Checkbox subscribe("Email me updates", 20, 372, 300, 24);

    sdlw::Label      planL("Plan", 20, 406);
    sdlw::RadioGroup plan(20, 426, 200, { "Free", "Pro", "Team" });

    sdlw::Label       progL("Progress", 20, 516);
    sdlw::ProgressBar bar(20, 536, 300, 18);
    bar.setShowPercent(true);

    sdlw::Button submit("Submit", 20, 566, 140, 34);
    sdlw::Button reset("Reset",  180, 566, 140, 34);

    // --- Column 2: lists -------------------------------------------------
    sdlw::Label   itemsL("Items (ListBox)", 350, 60);
    sdlw::ListBox items(350, 80, 300, 240);
    items.setItems({ "Alpha", "Bravo", "Charlie", "Delta", "Echo", "Foxtrot", "Golf",
                     "Hotel", "India", "Juliet", "Kilo", "Lima", "Mike", "November",
                     "Oscar", "Papa", "Quebec", "Romeo", "Sierra", "Tango" });
    items.setSelected(0);

    sdlw::Label      panelL("Scrollable panel (ScrollView)", 350, 340);
    sdlw::ScrollView view(350, 360, 300, 300);
    const int kLines = 28;
    view.setContentHeight(kLines * 24 + 12);

    // --- Column 3: table -------------------------------------------------
    sdlw::Label    tableL("People (ListView)", 680, 60);
    sdlw::ListView table(680, 80, 340, 400);
    // Click a column header to sort (unordered -> ascending -> descending).
    // Total column width (490) exceeds the widget (340), so a horizontal
    // scrollbar appears; Shift+wheel or the bottom bar scroll horizontally.
    table.setColumns({ { "Name", 180, sdlw::ListView::Align::Left },
                       { "Age", 70, sdlw::ListView::Align::Right, sdlw::ListView::SortType::Numeric },
                       { "City", 130, sdlw::ListView::Align::Left },
                       { "Role", 110, sdlw::ListView::Align::Left } });
    table.setRows({
        { "Ada Lovelace", "36", "London", "Math" },
        { "Alan Turing", "41", "Bletchley", "Crypto" },
        { "Grace Hopper", "45", "New York", "Navy" },
        { "Edsger Dijkstra", "40", "Austin", "CS" },
        { "Katherine J.", "44", "Hampton", "NASA" },
        { "Donald Knuth", "39", "Stanford", "Books" },
        { "Barbara Liskov", "33", "Boston", "MIT" },
        { "Linus Torvalds", "38", "Portland", "Linux" },
        { "Margaret H.", "42", "Boston", "Apollo" },
        { "Ken Thompson", "47", "Murray Hill", "Unix" },
        { "Dennis Ritchie", "48", "Murray Hill", "C" },
        { "John Carmack", "35", "Dallas", "Games" },
        { "Bjarne S.", "43", "Aarhus", "C++" },
        { "Guido van R.", "46", "Amsterdam", "Python" },
        { "Tim Berners-Lee", "50", "London", "Web" },
        { "Vint Cerf", "52", "Los Angeles", "TCP/IP" },
        { "Radia Perlman", "49", "Seattle", "STP" },
        { "James Gosling", "45", "Calgary", "Java" },
        { "Anders Hejlsberg", "48", "Seattle", "C#" },
        { "Brendan Eich", "44", "San Jose", "JS" },
        { "Rasmus Lerdorf", "43", "Toronto", "PHP" },
        { "Yukihiro M.", "47", "Matsue", "Ruby" },
        { "Rich Hickey", "51", "New York", "Clojure" },
        { "Joe Armstrong", "53", "Stockholm", "Erlang" },
        { "Simon P. Jones", "54", "Cambridge", "Haskell" },
        { "Chris Lattner", "37", "Cupertino", "Swift" },
        { "Andrew Kelley", "34", "NYC", "Zig" },
        { "Graydon Hoare", "45", "Vancouver", "Rust" },
    });
    table.setSelected(0);

    sdlw::Label status("", 680, 500);
    status.style().color[0] = 150; status.style().color[1] = 230; status.style().color[2] = 170;

    // --- Tab order -------------------------------------------------------
    sdlw::FocusManager focus;
    focus.add(&name); focus.add(&country); focus.add(&lang);
    focus.add(&subscribe); focus.add(&plan);
    focus.add(&items); focus.add(&view); focus.add(&table);
    focus.add(&submit); focus.add(&reset);

    float t = 0;
    while (win.pumpEvents()) {
        focus.update(win);
        name.update(win, ui);
        country.update(win, ui);
        lang.update(win, ui);
        subscribe.update(win);
        plan.update(win);
        items.update(win, ui);
        table.update(win, ui);
        view.update(win);
        if (submit.update(win)) {
            std::string who = name.text().empty() ? "(no name)" : name.text();
            status.setText("Submitted: " + who);
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

        // Column 1
        nameL.draw(win.renderer(), ui);      name.draw(win.renderer(), ui);
        countryL.draw(win.renderer(), ui);
        langL.draw(win.renderer(), ui);
        subscribe.draw(win.renderer(), ui);
        planL.draw(win.renderer(), ui);      plan.draw(win.renderer(), ui);
        progL.draw(win.renderer(), ui);      bar.draw(win.renderer(), ui);
        submit.draw(win.renderer(), ui);     reset.draw(win.renderer(), ui);

        // Column 2
        itemsL.draw(win.renderer(), ui);     items.draw(win.renderer(), ui);
        panelL.draw(win.renderer(), ui);
        view.beginContent(win.renderer());
        for (int i = 0; i < kLines; ++i) {
            char line[64];
            std::snprintf(line, sizeof line, "Log entry %d — scrollable content", i + 1);
            ui.draw(line, view.contentX(), view.contentTop() + i * 24, 210, 210, 220);
        }
        view.endContent(win.renderer());

        // Column 3
        tableL.draw(win.renderer(), ui);     table.draw(win.renderer(), ui);
        status.draw(win.renderer(), ui);

        // Dropdowns last so their popups sit on top.
        country.draw(win.renderer(), ui);
        lang.draw(win.renderer(), ui);

        focus.drawFocusRing(win.renderer());
        win.present();
    }
    return 0;
}
