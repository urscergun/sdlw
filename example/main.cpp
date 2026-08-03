// Example sdlw application: a scrollable ListBox plus a TextBox to add items,
// all from font atlases embedded in the executable.
//
// There is no WinMain/main here — sdlw supplies the platform entry point and
// calls Main().
#include "sdlw/window.h"
#include "sdlw/font.h"
#include "sdlw/button.h"
#include "sdlw/textbox.h"
#include "sdlw/listbox.h"

#include <cstdio>
#include <string>

// Font atlases + descriptors baked into the executable by CMake (tools/bin2c.cmake).
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

    sdlw::Window win({
        .title  = "sdlw list box demo",
        .width  = 560,
        .height = 360,
    });
    if (!win.ok()) {
        std::fprintf(stderr, "window: %s\n", win.error());
        return 1;
    }

    sdlw::Font heading, ui;
    if (!heading.loadFromMemory(win.renderer(), dejavusans_24_fnt, dejavusans_24_fnt_len,
                                dejavusans_24_bmp, dejavusans_24_bmp_len) ||
        !ui.loadFromMemory(win.renderer(), dejavusans_16_fnt, dejavusans_16_fnt_len,
                           dejavusans_16_bmp, dejavusans_16_bmp_len)) {
        std::fprintf(stderr, "font: %s\n", ui.error());
        return 1;
    }

    sdlw::ListBox list(20, 70, 300, 250);
    list.setItems({
        "Alpha", "Bravo", "Charlie", "Delta", "Echo", "Foxtrot", "Golf",
        "Hotel", "India", "Juliet", "Kilo", "Lima", "Mike", "November",
        "Oscar", "Papa", "Quebec", "Romeo", "Sierra", "Tango",
    });
    list.setSelected(0);

    sdlw::TextBox entry(340, 70, 200, 34);
    entry.setPlaceholder("New item...");
    sdlw::Button addBtn("Add", 340, 114, 200, 34);

    while (win.pumpEvents()) {
        list.update(win, ui);
        entry.update(win, ui);
        if (addBtn.update() && !entry.text().empty()) {
            list.addItem(entry.text());
            entry.setText("");
        }

        win.clear(24, 24, 32);
        heading.draw("sdlw list box", 20, 24, 120, 200, 255);

        list.draw(win.renderer(), ui);
        entry.draw(win.renderer(), ui);
        addBtn.draw(win.renderer(), ui);

        if (const std::string* sel = list.selectedItem()) {
            std::string s = "Selected: " + *sel;
            ui.draw(s.c_str(), 340, 170, 150, 230, 170);
        }
        char cnt[48];
        std::snprintf(cnt, sizeof cnt, "%d items", list.count());
        ui.draw(cnt, 340, 196, 170, 170, 185);

        win.present();
    }
    return 0;
}
