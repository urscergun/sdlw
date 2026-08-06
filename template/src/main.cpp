// Minimal sdlw application: a window with a label and a button.
//
// You write Main(); sdlw provides the platform entry point (WinMain on Windows,
// main on Linux) inside the library. The font is baked into the executable by
// the CMakeLists via sdlw_embed_asset.
#include "sdlw/window.h"
#include "sdlw/font.h"
#include "sdlw/label.h"
#include "sdlw/button.h"

#include <cstdio>

// Font atlas + descriptor embedded by CMake (see CMakeLists.txt).
extern "C" {
    extern const unsigned char font_16_fnt[];
    extern const unsigned int  font_16_fnt_len;
    extern const unsigned char font_16_bmp[];
    extern const unsigned int  font_16_bmp_len;
}

int Main(int argc, char** argv) {
    (void)argc; (void)argv;

    sdlw::Window win({ .title = "my sdlw app", .width = 400, .height = 200 });
    if (!win.ok()) { std::fprintf(stderr, "window: %s\n", win.error()); return 1; }

    sdlw::Font ui;
    if (!ui.loadFromMemory(win.renderer(), font_16_fnt, font_16_fnt_len,
                           font_16_bmp, font_16_bmp_len)) {
        std::fprintf(stderr, "font: %s\n", ui.error());
        return 1;
    }

    sdlw::Label  hello("Hello, sdlw!", 20, 20);
    sdlw::Button ok("Click me", 20, 60, 120, 36);
    int clicks = 0;

    while (win.pumpEvents()) {
        if (ok.update(win)) ++clicks;

        win.clear(24, 24, 32);
        hello.draw(win.renderer(), ui);
        ok.draw(win.renderer(), ui);
        char s[32];
        std::snprintf(s, sizeof s, "Clicks: %d", clicks);
        ui.draw(s, 20, 110, 200, 220, 180);
        win.present();
    }
    return 0;
}
