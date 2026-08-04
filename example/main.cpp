// Example sdlw application: a small form with keyboard focus + Tab traversal.
//
// Tab / Shift+Tab move focus between controls (a ring marks the focused one);
// type into fields, Space toggles the checkbox, arrow keys move the radio, and
// Enter/Space activates the button. Everything is embedded — no external files.
#include "sdlw/window.h"
#include "sdlw/font.h"
#include "sdlw/label.h"
#include "sdlw/textbox.h"
#include "sdlw/checkbox.h"
#include "sdlw/radiogroup.h"
#include "sdlw/button.h"
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

    sdlw::Window win({ .title = "sdlw form", .width = 420, .height = 380 });
    if (!win.ok()) { std::fprintf(stderr, "window: %s\n", win.error()); return 1; }

    sdlw::Font heading, ui;
    if (!heading.loadFromMemory(win.renderer(), dejavusans_24_fnt, dejavusans_24_fnt_len,
                                dejavusans_24_bmp, dejavusans_24_bmp_len) ||
        !ui.loadFromMemory(win.renderer(), dejavusans_16_fnt, dejavusans_16_fnt_len,
                           dejavusans_16_bmp, dejavusans_16_bmp_len)) {
        std::fprintf(stderr, "font: %s\n", ui.error());
        return 1;
    }

    sdlw::Label   title("Sign up", 20, 20);
    sdlw::Label   nameL("Name",  20, 64);
    sdlw::TextBox name(90, 60, 300, 30);
    sdlw::Label   mailL("Email", 20, 104);
    sdlw::TextBox email(90, 100, 300, 30);
    sdlw::Checkbox subscribe("Email me updates", 90, 146, 220, 24);
    sdlw::RadioGroup plan(90, 182, 200, { "Free", "Pro", "Team" });
    sdlw::Button  submit("Sign up", 90, 270, 120, 34);

    // Tab order.
    sdlw::FocusManager focus;
    focus.add(&name);
    focus.add(&email);
    focus.add(&subscribe);
    focus.add(&plan);
    focus.add(&submit);

    std::string status;

    while (win.pumpEvents()) {
        focus.update(win);          // Tab/Shift+Tab + click-to-focus, before widgets
        name.update(win, ui);
        email.update(win, ui);
        subscribe.update(win);
        plan.update(win);
        if (submit.update(win)) {
            status = "Signed up: " + (name.text().empty() ? std::string("(no name)") : name.text());
        }

        win.clear(24, 24, 32);
        title.style().color[0] = 120; title.style().color[1] = 200; title.style().color[2] = 255;
        title.draw(win.renderer(), heading);
        nameL.draw(win.renderer(), ui);
        mailL.draw(win.renderer(), ui);
        name.draw(win.renderer(), ui);
        email.draw(win.renderer(), ui);
        subscribe.draw(win.renderer(), ui);
        plan.draw(win.renderer(), ui);
        submit.draw(win.renderer(), ui);
        focus.drawFocusRing(win.renderer());   // ring on top of the focused control

        if (!status.empty())
            ui.draw(status.c_str(), 20, 326, 150, 230, 170);

        win.present();
    }
    return 0;
}
