// Example sdlw application: an editable ComboBox (text box + drop-down list),
// rendered from font atlases embedded in the executable.
//
// There is no WinMain/main here — sdlw supplies the platform entry point and
// calls Main().
#include "sdlw/window.h"
#include "sdlw/font.h"
#include "sdlw/combobox.h"

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
        .title  = "sdlw combo box demo",
        .width  = 460,
        .height = 300,
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

    sdlw::ComboBox combo(20, 96, 300, 34);
    combo.setItems({
        "Argentina", "Australia", "Austria", "Belgium", "Brazil", "Canada",
        "Chile", "China", "Denmark", "Egypt", "Finland", "France", "Germany",
        "Greece", "India", "Ireland", "Italy", "Japan", "Mexico", "Norway",
        "Poland", "Portugal", "Spain", "Sweden", "Switzerland",
    });
    combo.setText("");

    std::string chosen;

    while (win.pumpEvents()) {
        if (combo.update(win, ui)) chosen = combo.text();

        win.clear(24, 24, 32);
        heading.draw("sdlw combo box", 20, 24, 120, 200, 255);
        ui.draw("Country (type to filter):", 20, 72, 200, 200, 210);

        if (!chosen.empty()) {
            std::string s = "Chosen: " + chosen;
            ui.draw(s.c_str(), 20, 150, 150, 230, 170);
        }

        // Draw the combo LAST so its popup appears on top of everything else.
        combo.draw(win.renderer(), ui);

        win.present();
    }
    return 0;
}
