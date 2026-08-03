// Example sdlw application: text + clickable buttons from embedded assets.
//
// There is no WinMain/main here — sdlw supplies the platform entry point and
// calls Main(). Font atlases are embedded in the executable, so no external
// files are needed.
#include "sdlw/window.h"
#include "sdlw/font.h"
#include "sdlw/button.h"

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
        .title  = "sdlw button demo",
        .width  = 520,
        .height = 260,
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

    sdlw::Button clickBtn("Click me", 20, 90, 140, 44);
    sdlw::Button resetBtn("Reset",   176, 90,  90, 44);
    // Give reset a warmer tint.
    resetBtn.style().hover[0] = 120; resetBtn.style().hover[1] = 70; resetBtn.style().hover[2] = 70;

    int clicks = 0;

    while (win.pumpEvents()) {
        if (clickBtn.update()) ++clicks;
        if (resetBtn.update()) clicks = 0;

        win.clear(24, 24, 32);

        heading.draw("sdlw buttons", 20, 20, 120, 200, 255);

        clickBtn.draw(win.renderer(), ui);
        resetBtn.draw(win.renderer(), ui);

        char status[64];
        std::snprintf(status, sizeof status, "Clicks: %d", clicks);
        ui.draw(status, 20, 160, 230, 230, 235);

        win.present();
    }
    return 0;
}
