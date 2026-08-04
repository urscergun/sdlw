// Example sdlw application: a small gallery of controls — Label, Checkbox,
// RadioGroup, ProgressBar, and a (non-editable) Select — from embedded fonts.
//
// There is no WinMain/main here — sdlw supplies the platform entry point and
// calls Main().
#include "sdlw/window.h"
#include "sdlw/font.h"
#include "sdlw/label.h"
#include "sdlw/checkbox.h"
#include "sdlw/radiogroup.h"
#include "sdlw/progressbar.h"
#include "sdlw/select.h"

#include <cstdio>

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

    sdlw::Window win({ .title = "sdlw controls", .width = 460, .height = 380 });
    if (!win.ok()) { std::fprintf(stderr, "window: %s\n", win.error()); return 1; }

    sdlw::Font heading, ui;
    if (!heading.loadFromMemory(win.renderer(), dejavusans_24_fnt, dejavusans_24_fnt_len,
                                dejavusans_24_bmp, dejavusans_24_bmp_len) ||
        !ui.loadFromMemory(win.renderer(), dejavusans_16_fnt, dejavusans_16_fnt_len,
                           dejavusans_16_bmp, dejavusans_16_bmp_len)) {
        std::fprintf(stderr, "font: %s\n", ui.error());
        return 1;
    }

    sdlw::Label      title("sdlw controls", 20, 20);
    sdlw::Checkbox   agree("Enable notifications", 20, 80, 240, 24);
    sdlw::RadioGroup theme(20, 120, 200, { "Light", "Dark", "System" });
    sdlw::ProgressBar bar(20, 220, 300, 18);
    bar.setShowPercent(true);
    bar.setValue(0.35f);
    sdlw::Select     lang(20, 270, 240, 30);
    lang.setPlaceholder("Choose a language...");
    lang.setItems({ "C", "C++", "Python", "Rust", "Go", "JavaScript", "Zig", "Lua" });

    float t = 0;
    while (win.pumpEvents()) {
        agree.update(win);
        theme.update(win);
        lang.update(win, ui);
        t += 0.004f; if (t > 1.0f) t = 0.0f;   // animate the progress bar
        bar.setValue(t);

        win.clear(24, 24, 32);
        title.style().color[0] = 120; title.style().color[1] = 200; title.style().color[2] = 255;
        title.draw(win.renderer(), heading);

        agree.draw(win.renderer(), ui);
        theme.draw(win.renderer(), ui);
        bar.draw(win.renderer(), ui);

        // Draw the Select LAST so its popup sits on top.
        lang.draw(win.renderer(), ui);

        win.present();
    }
    return 0;
}
