// Example sdlw application.
//
// Note there is no WinMain/main here — just Main(). sdlw supplies the
// platform entry point and calls this.
#include "sdlw/window.h"

#include <cstdio>

int Main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    sdlw::Window win({
        .title  = "sdlw example",
        .width  = 1024,
        .height = 640,
    });

    if (!win.ok()) {
        std::fprintf(stderr, "failed to create window: %s\n", win.error());
        return 1;
    }

    // Simple animated background so we can see it's alive.
    unsigned char t = 0;
    while (win.pumpEvents()) {
        win.clear(t, 40, 80 - t / 4);
        win.present();
        ++t;
    }

    return 0;
}
