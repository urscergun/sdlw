#include "sdlw/window.h"

// We manage the entry point ourselves (see entry.cpp). SDL3's SDL.h does not
// hijack main(), but we define this and call SDL_SetMainReady() to be explicit.
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // for SDL_SetMainReady(); SDL_MAIN_HANDLED keeps main() ours

#include <string>

namespace sdlw {

// Reference-counted SDL_Init so multiple Windows share one subsystem lifetime.
namespace {
int  g_initCount = 0;
bool ensureSdlInit(std::string& err) {
    if (g_initCount == 0) {
        SDL_SetMainReady();
        if (!SDL_Init(SDL_INIT_VIDEO)) { // SDL3 returns bool: true on success.
            err = SDL_GetError();
            return false;
        }
    }
    ++g_initCount;
    return true;
}
void releaseSdlInit() {
    if (g_initCount > 0 && --g_initCount == 0) {
        SDL_Quit();
    }
}
} // namespace

struct Window::Impl {
    SDL_Window*   window = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::string   error;
    bool          sdlOwned = false;

    // Per-frame input, refreshed by pumpEvents().
    std::string   frameText;                        // UTF-8 typed this frame
    bool          keys[int(Key::Count)] = {};       // indexed by Key enum
    bool          mousePressed = false;             // left button down this frame
    int           mouseClicks = 0;                  // click count of that press
    float         mouseWheel = 0;                   // vertical wheel delta this frame

    // Sampled input state (level), refreshed by pumpEvents() or feed* calls.
    float         mouseX = 0, mouseY = 0;
    bool          mouseDown = false;
    bool          modShift = false, modCtrl = false;
    std::string   clipboard;                        // in-memory clipboard (headless)

    std::function<void()> frameCb;                  // render-one-frame callback
    bool          watchAdded = false;               // event watch installed
};

namespace {
// Map an SDL key event to a sdlw::Key index, or -1 if not one we track.
// Ctrl+A/C/X/V map to the Select/Copy/Cut/Paste command entries.
int keyIndex(SDL_Keycode k, SDL_Keymod mod) {
    if (mod & SDL_KMOD_CTRL) {
        switch (k) {
            case SDLK_A: return int(Key::SelectAll);
            case SDLK_C: return int(Key::Copy);
            case SDLK_X: return int(Key::Cut);
            case SDLK_V: return int(Key::Paste);
            default: break;
        }
    }
    switch (k) {
        case SDLK_BACKSPACE: return int(Key::Backspace);
        case SDLK_DELETE:    return int(Key::Delete);
        case SDLK_LEFT:      return int(Key::Left);
        case SDLK_RIGHT:     return int(Key::Right);
        case SDLK_UP:        return int(Key::Up);
        case SDLK_DOWN:      return int(Key::Down);
        case SDLK_HOME:      return int(Key::Home);
        case SDLK_END:       return int(Key::End);
        case SDLK_PAGEUP:    return int(Key::PageUp);
        case SDLK_PAGEDOWN:  return int(Key::PageDown);
        case SDLK_RETURN:    return int(Key::Enter);
        case SDLK_TAB:       return int(Key::Tab);
        case SDLK_SPACE:     return int(Key::Space);
        case SDLK_F1:  return int(Key::F1);   case SDLK_F2:  return int(Key::F2);
        case SDLK_F3:  return int(Key::F3);   case SDLK_F4:  return int(Key::F4);
        case SDLK_F5:  return int(Key::F5);   case SDLK_F6:  return int(Key::F6);
        case SDLK_F7:  return int(Key::F7);   case SDLK_F8:  return int(Key::F8);
        case SDLK_F9:  return int(Key::F9);   case SDLK_F10: return int(Key::F10);
        case SDLK_F11: return int(Key::F11);  case SDLK_F12: return int(Key::F12);
        default:             return -1;
    }
}
} // namespace

Window::Window(const WindowConfig& config) : impl_(new Impl) {
    if (!ensureSdlInit(impl_->error)) {
        return;
    }
    impl_->sdlOwned = true;

    // SDL3 windows are shown by default; flags are SDL_WindowFlags (64-bit).
    SDL_WindowFlags flags = 0;
    if (config.resizable) flags |= SDL_WINDOW_RESIZABLE;

    // SDL3 SDL_CreateWindow takes no position (title, w, h, flags).
    impl_->window = SDL_CreateWindow(config.title.c_str(),
                                     config.width, config.height, flags);
    if (!impl_->window) {
        impl_->error = SDL_GetError();
        return;
    }

    // SDL3 SDL_CreateRenderer takes (window, name); NULL picks the best driver.
    impl_->renderer = SDL_CreateRenderer(impl_->window, nullptr);
    if (!impl_->renderer) {
        impl_->error = SDL_GetError();
        return;
    }

    // VSync is configured separately in SDL3.
    SDL_SetRenderVSync(impl_->renderer,
                       config.vsync ? 1 : SDL_RENDERER_VSYNC_DISABLED);
}

// Headless: no SDL window/renderer/init. Used to drive widget logic in tests;
// input is supplied via the feed* methods instead of pumpEvents().
Window::Window(Headless) : impl_(new Impl) {}

Window::~Window() {
    if (impl_->watchAdded) SDL_RemoveEventWatch(&Window::frameWatch, impl_);
    if (impl_->renderer) SDL_DestroyRenderer(impl_->renderer);
    if (impl_->window)   SDL_DestroyWindow(impl_->window);
    if (impl_->sdlOwned) releaseSdlInit();
    delete impl_;
}

// Called by SDL for every event, including during the OS modal resize loop,
// which is where the main thread would otherwise be blocked. Redrawing on
// EXPOSED/RESIZED keeps the window live while it is being resized.
bool Window::frameWatch(void* userdata, SDL_Event* e) {
    Impl* impl = static_cast<Impl*>(userdata);
    if (impl && impl->frameCb &&
        (e->type == SDL_EVENT_WINDOW_EXPOSED || e->type == SDL_EVENT_WINDOW_RESIZED)) {
        if (!impl->window || e->window.windowID == SDL_GetWindowID(impl->window))
            impl->frameCb();
    }
    return true;   // keep the event in the queue
}

void Window::setFrameCallback(std::function<void()> render) {
    impl_->frameCb = std::move(render);
    if (impl_->window && !impl_->watchAdded) {
        SDL_AddEventWatch(&Window::frameWatch, impl_);
        impl_->watchAdded = true;
    }
}

bool Window::ok() const {
    return impl_->window != nullptr && impl_->renderer != nullptr;
}

bool Window::pumpEvents() {
    // Reset per-frame input.
    impl_->frameText.clear();
    for (bool& k : impl_->keys) k = false;
    impl_->mousePressed = false;
    impl_->mouseClicks = 0;
    impl_->mouseWheel = 0;

    bool running = true;
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_EVENT_QUIT:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                running = false;
                break;
            case SDL_EVENT_KEY_DOWN: {
                if (ev.key.key == SDLK_ESCAPE) { running = false; break; }
                int idx = keyIndex(ev.key.key, ev.key.mod);
                if (idx >= 0) impl_->keys[idx] = true;
                break;
            }
            case SDL_EVENT_TEXT_INPUT:
                if (ev.text.text) impl_->frameText += ev.text.text;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (ev.button.button == SDL_BUTTON_LEFT) {
                    impl_->mousePressed = true;
                    impl_->mouseClicks = ev.button.clicks;
                }
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                impl_->mouseWheel += ev.wheel.y;
                break;
            default:
                break;
        }
    }

    // Sample level input state once per frame.
    float mxx = 0, myy = 0;
    SDL_MouseButtonFlags mb = SDL_GetMouseState(&mxx, &myy);
    impl_->mouseX = mxx;
    impl_->mouseY = myy;
    impl_->mouseDown = (mb & SDL_BUTTON_LMASK) != 0;
    SDL_Keymod mod = SDL_GetModState();
    impl_->modShift = (mod & SDL_KMOD_SHIFT) != 0;
    impl_->modCtrl  = (mod & SDL_KMOD_CTRL) != 0;

    return running;
}

void Window::startTextInput() {
    if (impl_->window) SDL_StartTextInput(impl_->window);
}

void Window::stopTextInput() {
    if (impl_->window) SDL_StopTextInput(impl_->window);
}

const char* Window::textInput() const { return impl_->frameText.c_str(); }

bool Window::keyPressed(Key key) const {
    int idx = int(key);
    return (idx >= 0 && idx < int(Key::Count)) ? impl_->keys[idx] : false;
}

bool  Window::mousePressed() const { return impl_->mousePressed; }
int   Window::mouseClicks() const { return impl_->mouseClicks; }
float Window::mouseWheel() const { return impl_->mouseWheel; }
float Window::mouseX() const { return impl_->mouseX; }
float Window::mouseY() const { return impl_->mouseY; }
bool  Window::mouseDown() const { return impl_->mouseDown; }
bool  Window::modShift() const { return impl_->modShift; }
bool  Window::modCtrl() const { return impl_->modCtrl; }

std::string Window::clipboardText() const {
    if (impl_->window) {
        char* c = SDL_GetClipboardText();
        std::string s = c ? c : "";
        if (c) SDL_free(c);
        return s;
    }
    return impl_->clipboard;
}

void Window::setClipboardText(const std::string& text) {
    impl_->clipboard = text;
    if (impl_->window) SDL_SetClipboardText(text.c_str());
}

// --- Headless input injection (tests) --------------------------------------
void Window::clearFrameInput() {
    impl_->frameText.clear();
    for (bool& k : impl_->keys) k = false;
    impl_->mousePressed = false;
    impl_->mouseClicks = 0;
    impl_->mouseWheel = 0;
}
void Window::feedMouse(float x, float y, bool down) {
    impl_->mouseX = x; impl_->mouseY = y; impl_->mouseDown = down;
}
void Window::feedMods(bool shift, bool ctrl) { impl_->modShift = shift; impl_->modCtrl = ctrl; }
void Window::feedMousePress(int clicks) { impl_->mousePressed = true; impl_->mouseClicks = clicks; }
void Window::feedWheel(float delta) { impl_->mouseWheel += delta; }
void Window::feedText(const std::string& utf8) { impl_->frameText += utf8; }
void Window::feedKey(Key key) {
    int idx = int(key);
    if (idx >= 0 && idx < int(Key::Count)) impl_->keys[idx] = true;
}

void Window::clear(unsigned char r, unsigned char g, unsigned char b) {
    if (!impl_->renderer) return;
    SDL_SetRenderDrawColor(impl_->renderer, r, g, b, 255);
    SDL_RenderClear(impl_->renderer);
}

void Window::present() {
    if (impl_->renderer) SDL_RenderPresent(impl_->renderer);
}

int Window::width() const {
    int w = 0, h = 0;
    if (impl_->window) SDL_GetWindowSize(impl_->window, &w, &h);
    return w;
}

int Window::height() const {
    int w = 0, h = 0;
    if (impl_->window) SDL_GetWindowSize(impl_->window, &w, &h);
    return h;
}

SDL_Window*   Window::handle() const   { return impl_->window; }
SDL_Renderer* Window::renderer() const { return impl_->renderer; }

const char* Window::error() const { return impl_->error.c_str(); }

} // namespace sdlw
