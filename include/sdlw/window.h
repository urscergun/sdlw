// sdlw - a small cross-platform GUI window API built on libSDL.
//
// The framework owns the platform entry point (WinMain on Windows, main on
// Linux) and calls into the single function you provide:
//
//     int Main(int argc, char** argv);
//
// Everything else here is plain in-house C++; libSDL is the only dependency.
#pragma once

#include <string>

// Opaque SDL forward declarations so users don't need SDL headers to use the API.
struct SDL_Window;
struct SDL_Renderer;

namespace sdlw {

// Editing keys / command combos reported per-frame by Window (see keyPressed()).
// The Select/Copy/Cut/Paste entries are the Ctrl+A/C/X/V shortcuts.
enum class Key {
    Backspace, Delete, Left, Right, Up, Down, Home, End, Enter, Tab,
    SelectAll, Copy, Cut, Paste,
    Count
};

struct WindowConfig {
    std::string title = "sdlw window";
    int         width = 800;
    int         height = 600;
    bool        resizable = true;
    bool        vsync = true;
};

// A single top-level GUI window plus an accelerated renderer.
//
// Non-copyable, movable-by-nothing (owns SDL resources). Construct one, check
// ok(), then drive it from a loop:
//
//     sdlw::Window win({.title = "Hello"});
//     if (!win.ok()) return 1;
//     while (win.pumpEvents()) {
//         win.clear(20, 20, 28);
//         // ... in-house drawing against win.renderer() ...
//         win.present();
//     }
class Window {
public:
    explicit Window(const WindowConfig& config = {});
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // True if the window and renderer were created successfully.
    bool ok() const;

    // Drain the OS event queue. Returns false once a quit is requested
    // (window closed or Escape pressed), true to keep running.
    bool pumpEvents();

    // Fill the back buffer with an RGB color.
    void clear(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0);

    // Swap the back buffer to the screen.
    void present();

    int width() const;
    int height() const;

    // --- Text/keyboard input -----------------------------------------------
    // Enable/disable OS text input (IME-aware). A text widget calls these as it
    // gains/loses focus so that textInput() starts/stops delivering characters.
    void startTextInput();
    void stopTextInput();

    // UTF-8 text typed during the most recent pumpEvents() (empty if none).
    const char* textInput() const;

    // True if `key` was pressed (or auto-repeated) during the most recent
    // pumpEvents(). Edge/repeat-triggered, for editing controls.
    bool keyPressed(Key key) const;

    // Left mouse button press this frame (edge), and its click count
    // (1 = single, 2 = double, ...). mouseClicks() is 0 when no press occurred.
    bool mousePressed() const;
    int  mouseClicks() const;

    // Vertical mouse-wheel delta accumulated during the most recent
    // pumpEvents(). Positive = scrolled away from the user (up).
    float mouseWheel() const;

    // Low-level handles for in-house rendering code.
    SDL_Window*   handle() const;
    SDL_Renderer* renderer() const;

    // Human-readable description of the last failure, or "" if none.
    const char* error() const;

private:
    struct Impl;
    Impl* impl_;
};

} // namespace sdlw

// You implement this. sdlw provides the platform-specific entry point that
// initializes SDL and calls it.
int Main(int argc, char** argv);
