// Example sdlw application: text, a clickable button, and an editable text box,
// all from font atlases embedded in the executable.
//
// There is no WinMain/main here — sdlw supplies the platform entry point and
// calls Main().
#include "sdlw/window.h"
#include "sdlw/font.h"
#include "sdlw/button.h"
#include "sdlw/textbox.h"

#include <cstdio>

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
        .title  = "sdlw text box demo",
        .width  = 520,
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

    sdlw::TextBox name(20, 96, 360, 34);
    name.setPlaceholder("Type your name...");

    sdlw::Button greetBtn("Greet", 396, 96, 100, 34);

    std::string greeting;

    while (win.pumpEvents()) {
        name.update(win);
        if (greetBtn.update()) {
            greeting = name.text().empty() ? "" : ("Hello, " + name.text() + "!");
        }

        win.clear(24, 24, 32);

        heading.draw("sdlw text box", 20, 24, 120, 200, 255);
        ui.draw("Name:", 20, 72, 200, 200, 210);

        name.draw(win.renderer(), ui);
        greetBtn.draw(win.renderer(), ui);

        if (!greeting.empty())
            ui.draw(greeting.c_str(), 20, 160, 150, 230, 170);

        win.present();
    }
    return 0;
}
