// Example sdlw application: every widget, arranged with the layout engine.
//
// Instead of hand-placed pixel coordinates, widgets are put into VBox/HBox
// containers and given Fixed/Flex sizes; the tree is arranged into the window
// each frame, so the UI reflows when the window is resized. Tab moves focus;
// all fonts are embedded. sdlw provides the entry point and calls Main().
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
#include "sdlw/layout.h"

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

    sdlw::Window win({ .title = "sdlw — all widgets (layout)", .width = 1040, .height = 700 });
    if (!win.ok()) { std::fprintf(stderr, "window: %s\n", win.error()); return 1; }

    sdlw::Font heading, ui;
    if (!heading.loadFromMemory(win.renderer(), dejavusans_24_fnt, dejavusans_24_fnt_len,
                                dejavusans_24_bmp, dejavusans_24_bmp_len) ||
        !ui.loadFromMemory(win.renderer(), dejavusans_16_fnt, dejavusans_16_fnt_len,
                           dejavusans_16_bmp, dejavusans_16_bmp_len)) {
        std::fprintf(stderr, "font: %s\n", ui.error());
        return 1;
    }

    using namespace sdlw;
    Label title("sdlw — all widgets (layout)", 0, 0);

    // Column 1: form.
    Label nameL("Name", 0, 0);
    TextBox name; name.setPlaceholder("Type your name...");
    Label countryL("Country (combo)", 0, 0);
    ComboBox country; country.setMaxVisibleRows(4);
    country.setItems({ "Argentina", "Australia", "Brazil", "Canada", "China", "Denmark",
                       "Egypt", "France", "Germany", "India", "Italy", "Japan", "Mexico",
                       "Norway", "Poland", "Spain", "Sweden", "Switzerland" });
    Label langL("Language (select)", 0, 0);
    Select lang; lang.setMaxVisibleRows(4); lang.setPlaceholder("Choose...");
    lang.setItems({ "C", "C++", "Python", "Rust", "Go", "Zig", "Lua" });
    Checkbox subscribe("Email me updates", 0, 0, 0, 24);
    Label planL("Plan", 0, 0);
    RadioGroup plan(0, 0, 0, { "Free", "Pro", "Team" });
    Label progL("Progress", 0, 0);
    ProgressBar bar; bar.setShowPercent(true);
    Button submit("Submit", 0, 0, 0, 0);
    Button reset("Reset", 0, 0, 0, 0);

    // Column 2: lists.
    Label itemsL("Items (ListBox)", 0, 0);
    ListBox items;
    items.setItems({ "Alpha", "Bravo", "Charlie", "Delta", "Echo", "Foxtrot", "Golf",
                     "Hotel", "India", "Juliet", "Kilo", "Lima", "Mike", "November",
                     "Oscar", "Papa", "Quebec", "Romeo", "Sierra", "Tango" });
    items.setSelected(0);
    Label panelL("Scrollable panel (ScrollView)", 0, 0);
    ScrollView view;
    const int kLines = 30;
    view.setContentHeight(kLines * 24 + 12);

    // Column 3: table.
    Label tableL("People (ListView — click headers to sort)", 0, 0);
    ListView table;
    table.setColumns({ { "Name", 180, ListView::Align::Left },
                       { "Age", 70, ListView::Align::Right, ListView::SortType::Numeric },
                       { "City", 130, ListView::Align::Left },
                       { "Role", 110, ListView::Align::Left } });
    table.setRows({
        { "Ada Lovelace", "36", "London", "Math" }, { "Alan Turing", "41", "Bletchley", "Crypto" },
        { "Grace Hopper", "45", "New York", "Navy" }, { "Edsger Dijkstra", "40", "Austin", "CS" },
        { "Katherine J.", "44", "Hampton", "NASA" }, { "Donald Knuth", "39", "Stanford", "Books" },
        { "Barbara Liskov", "33", "Boston", "MIT" }, { "Linus Torvalds", "38", "Portland", "Linux" },
        { "Margaret H.", "42", "Boston", "Apollo" }, { "Ken Thompson", "47", "Murray Hill", "Unix" },
        { "Dennis Ritchie", "48", "Murray Hill", "C" }, { "John Carmack", "35", "Dallas", "Games" },
        { "Bjarne S.", "43", "Aarhus", "C++" }, { "Guido van R.", "46", "Amsterdam", "Python" },
        { "Tim Berners-Lee", "50", "London", "Web" }, { "Vint Cerf", "52", "Los Angeles", "TCP/IP" },
        { "Radia Perlman", "49", "Seattle", "STP" }, { "James Gosling", "45", "Calgary", "Java" },
        { "Brendan Eich", "44", "San Jose", "JS" }, { "Yukihiro M.", "47", "Matsue", "Ruby" },
    });
    table.setSelected(0);
    Label status("", 0, 0);
    status.style().color[0] = 150; status.style().color[1] = 230; status.style().color[2] = 170;

    // --- Layout tree ------------------------------------------------------
    HBox buttons({ .spacing = 8 });
    buttons.add(submit, Size::flex(1));
    buttons.add(reset,  Size::flex(1));

    VBox col1({ .spacing = 6 });
    col1.add(nameL, Size::fixed(20));     col1.add(name, Size::fixed(30));
    col1.add(countryL, Size::fixed(20));  col1.add(country, Size::fixed(30));
    col1.add(langL, Size::fixed(20));     col1.add(lang, Size::fixed(30));
    col1.add(subscribe, Size::fixed(24));
    col1.add(planL, Size::fixed(20));     col1.add(plan, Size::fixed(78));
    col1.add(progL, Size::fixed(20));     col1.add(bar, Size::fixed(18));
    col1.add(buttons, Size::fixed(34));
    col1.addSpacer(Size::flex());         // push the form up

    VBox col2({ .spacing = 6 });
    col2.add(itemsL, Size::fixed(20));    col2.add(items, Size::flex(1));
    col2.add(panelL, Size::fixed(20));    col2.add(view, Size::flex(1));

    VBox col3({ .spacing = 6 });
    col3.add(tableL, Size::fixed(20));    col3.add(table, Size::flex(1));
    col3.add(status, Size::fixed(24));

    HBox columns({ .spacing = 20 });
    columns.add(col1, Size::flex(1));
    columns.add(col2, Size::flex(1));
    columns.add(col3, Size::flex(1.3f));

    VBox root({ .padding = 16, .spacing = 12 });
    root.add(title, Size::fixed(32));
    root.add(columns, Size::flex(1));

    // Tab order.
    FocusManager focus;
    focus.add(&name); focus.add(&country); focus.add(&lang);
    focus.add(&subscribe); focus.add(&plan);
    focus.add(&items); focus.add(&view); focus.add(&table);
    focus.add(&submit); focus.add(&reset);

    // Render one frame: arrange the tree to the window size, then draw. Used
    // each loop iteration AND during live resize (registered below), so the UI
    // reflows and repaints while the window border is being dragged.
    auto render = [&]() {
        root.arrange({ 0, 0, float(win.width()), float(win.height()) });

        win.clear(24, 24, 32);
        title.style().color[0] = 120; title.style().color[1] = 200; title.style().color[2] = 255;
        title.draw(win.renderer(), heading);

        nameL.draw(win.renderer(), ui);    name.draw(win.renderer(), ui);
        countryL.draw(win.renderer(), ui);
        langL.draw(win.renderer(), ui);
        subscribe.draw(win.renderer(), ui);
        planL.draw(win.renderer(), ui);    plan.draw(win.renderer(), ui);
        progL.draw(win.renderer(), ui);    bar.draw(win.renderer(), ui);
        submit.draw(win.renderer(), ui);   reset.draw(win.renderer(), ui);

        itemsL.draw(win.renderer(), ui);   items.draw(win.renderer(), ui);
        panelL.draw(win.renderer(), ui);
        view.beginContent(win.renderer());
        for (int i = 0; i < kLines; ++i) {
            char line[64];
            std::snprintf(line, sizeof line, "Log entry %d — scrollable content", i + 1);
            ui.draw(line, view.contentX(), view.contentTop() + i * 24, 210, 210, 220);
        }
        view.endContent(win.renderer());

        tableL.draw(win.renderer(), ui);   table.draw(win.renderer(), ui);
        status.draw(win.renderer(), ui);

        country.draw(win.renderer(), ui);  // dropdowns last (popups on top)
        lang.draw(win.renderer(), ui);

        focus.drawFocusRing(win.renderer());
        win.present();
    };
    win.setFrameCallback(render);   // keep redrawing during live resize

    float t = 0;
    while (win.pumpEvents()) {
        root.arrange({ 0, 0, float(win.width()), float(win.height()) });
        focus.update(win);
        name.update(win, ui);   country.update(win, ui); lang.update(win, ui);
        subscribe.update(win);  plan.update(win);
        items.update(win, ui);  table.update(win, ui);   view.update(win);
        if (submit.update(win)) status.setText("Submitted: " + (name.text().empty() ? std::string("(no name)") : name.text()));
        if (reset.update(win)) {
            name.setText(""); country.setText(""); lang.setSelected(-1);
            subscribe.setChecked(false); plan.setSelected(0); status.setText("");
        }
        t += 0.004f; if (t > 1.0f) t = 0.0f;
        bar.setValue(t);

        render();
    }
    return 0;
}
