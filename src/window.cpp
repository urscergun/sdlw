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
};

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

Window::~Window() {
    if (impl_->renderer) SDL_DestroyRenderer(impl_->renderer);
    if (impl_->window)   SDL_DestroyWindow(impl_->window);
    if (impl_->sdlOwned) releaseSdlInit();
    delete impl_;
}

bool Window::ok() const {
    return impl_->window != nullptr && impl_->renderer != nullptr;
}

bool Window::pumpEvents() {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_EVENT_QUIT:
                return false;
            case SDL_EVENT_KEY_DOWN:
                if (ev.key.key == SDLK_ESCAPE) return false;
                break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                return false;
            default:
                break;
        }
    }
    return true;
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
