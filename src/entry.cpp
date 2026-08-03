// Platform entry points for sdlw.
//
// The user writes `int Main(int argc, char** argv)`. This file provides the
// real OS entry point and forwards to it:
//   - Windows: WinMain (GUI subsystem, no console window)
//   - Linux/other POSIX: main
#include "sdlw/window.h"

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

// __argc / __argv are provided by the CRT and hold the parsed command line,
// so we can reuse the same argc/argv contract as POSIX.
int WINAPI WinMain(HINSTANCE /*hInstance*/,
                   HINSTANCE /*hPrevInstance*/,
                   LPSTR     /*lpCmdLine*/,
                   int       /*nCmdShow*/) {
    return Main(__argc, __argv);
}

#else // POSIX (Linux, etc.)

int main(int argc, char** argv) {
    return Main(argc, argv);
}

#endif
