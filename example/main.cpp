// Example sdlw application: render multi-line text from a baked bitmap font.
//
// There is no WinMain/main here — sdlw supplies the platform entry point and
// calls Main(). Pass the .fnt path as argv[1], or it defaults to the baked
// DejaVu Sans atlas under assets/.
#include "sdlw/window.h"
#include "sdlw/font.h"

#include <cstdio>

int Main(int argc, char** argv) {
    const char* fontPath = (argc > 1) ? argv[1] : "assets/dejavusans_14.fnt";

    sdlw::Window win({
        .title  = "sdlw text demo",
        .width  = 640,
        .height = 360,
    });
    if (!win.ok()) {
        std::fprintf(stderr, "window: %s\n", win.error());
        return 1;
    }

    sdlw::Font font;
    if (!font.load(win.renderer(), fontPath)) {
        std::fprintf(stderr, "font: %s\n", font.error());
        std::fprintf(stderr, "(pass the .fnt path as the first argument, or run from the project root)\n");
        return 1;
    }

    const char* heading = "sdlw — bitmap font demo";
    const char* body =
        "DejaVu Sans, baked offline to a BMP atlas at 14px.\n"
        "The runtime links only SDL; glyphs are blitted from\n"
        "the atlas using the .fnt metrics.\n"
        "\n"
        "The quick brown fox jumps over the lazy dog.\n"
        "0123456789  !@#$%^&*()_+-=[]{};:'\",.<>/?\\|`~\n"
        "Ascenders/descenders: Ag Bj Ky Qp  |  tint below:";

    while (win.pumpEvents()) {
        win.clear(24, 24, 32);

        int x = 20, y = 20;
        font.draw(heading, float(x), float(y), 120, 200, 255);        // light blue
        y += font.lineHeight() + 6;
        font.draw(body, float(x), float(y), 230, 230, 235);           // near-white

        // A few tinted lines to show color-mod tinting from one atlas.
        int ty = y + 7 * font.lineHeight() + 6;
        font.draw("Red line",    float(x),       float(ty), 235,  90,  90);
        font.draw("Green line",  float(x + 110), float(ty),  90, 210, 120);
        font.draw("Amber line",  float(x + 230), float(ty), 240, 190,  70);

        win.present();
    }
    return 0;
}
