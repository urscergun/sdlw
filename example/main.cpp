// Example sdlw application: render text at several baked font sizes.
//
// There is no WinMain/main here — sdlw supplies the platform entry point and
// calls Main(). All font atlases are embedded in the executable, so no external
// files are needed.
#include "sdlw/window.h"
#include "sdlw/font.h"

#include <cstdio>

// Font atlases + descriptors baked into the executable by CMake (tools/bin2c.cmake).
#define SDLW_DECL_FONT(sz)                                     \
    extern "C" {                                               \
        extern const unsigned char dejavusans_##sz##_fnt[];    \
        extern const unsigned int  dejavusans_##sz##_fnt_len;  \
        extern const unsigned char dejavusans_##sz##_bmp[];    \
        extern const unsigned int  dejavusans_##sz##_bmp_len;  \
    }
SDLW_DECL_FONT(14)
SDLW_DECL_FONT(16)
SDLW_DECL_FONT(20)
SDLW_DECL_FONT(24)
SDLW_DECL_FONT(32)

int Main(int argc, char** argv) {
    (void)argc; (void)argv;

    sdlw::Window win({
        .title  = "sdlw text demo",
        .width  = 720,
        .height = 420,
    });
    if (!win.ok()) {
        std::fprintf(stderr, "window: %s\n", win.error());
        return 1;
    }

    // Load each embedded size into its own Font.
    struct Sized { int size; sdlw::Font font;
                   const unsigned char* fnt; unsigned int fntLen;
                   const unsigned char* bmp; unsigned int bmpLen; };
#define SDLW_ENTRY(sz) { sz, {}, dejavusans_##sz##_fnt, dejavusans_##sz##_fnt_len, \
                                dejavusans_##sz##_bmp, dejavusans_##sz##_bmp_len }
    Sized fonts[] = {
        SDLW_ENTRY(32), SDLW_ENTRY(24), SDLW_ENTRY(20),
        SDLW_ENTRY(16), SDLW_ENTRY(14),
    };
    for (auto& f : fonts) {
        if (!f.font.loadFromMemory(win.renderer(), f.fnt, f.fntLen, f.bmp, f.bmpLen)) {
            std::fprintf(stderr, "font %dpx: %s\n", f.size, f.font.error());
            return 1;
        }
    }

    while (win.pumpEvents()) {
        win.clear(24, 24, 32);

        // Heading in the largest size.
        int x = 20, y = 16;
        fonts[0].font.draw("sdlw — DejaVu Sans", float(x), float(y), 120, 200, 255);
        y += fonts[0].font.lineHeight() + 10;

        // One line per size, labelled, showing the crispness at each.
        char line[96];
        for (auto& f : fonts) {
            std::snprintf(line, sizeof line,
                          "%dpx  The quick brown fox jumps over the lazy dog", f.size);
            f.font.draw(line, float(x), float(y), 230, 230, 235);
            y += f.font.lineHeight() + 4;
        }

        win.present();
    }
    return 0;
}
