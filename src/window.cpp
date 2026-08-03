#include "sdlw/window.h"

// We manage the entry point ourselves (see entry.cpp), so SDL must not
// redefine main() behind our back.
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <string>

namespace sdlw {

// Reference-counted SDL_Init so multiple Windows share one subsystem lifetime.
namespace {
int  g_initCount = 0;
bool ensureSdlInit(std::string& err) {
    if (g_initCount == 0) {
        SDL_SetMainReady();
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
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

    Uint32 flags = SDL_WINDOW_SHOWN;
    if (config.resizable) flags |= SDL_WINDOW_RESIZABLE;

    impl_->window = SDL_CreateWindow(
        config.title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        config.width, config.height, flags);
    if (!impl_->window) {
        impl_->error = SDL_GetError();
        return;
    }

    Uint32 rflags = SDL_RENDERER_ACCELERATED;
    if (config.vsync) rflags |= SDL_RENDERER_PRESENTVSYNC;

    impl_->renderer = SDL_CreateRenderer(impl_->window, -1, rflags);
    if (!impl_->renderer) {
        impl_->error = SDL_GetError();
        return;
    }
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
            case SDL_QUIT:
                return false;
            case SDL_KEYDOWN:
                if (ev.key.keysym.sym == SDLK_ESCAPE) return false;
                break;
            case SDL_WINDOWEVENT:
                if (ev.window.event == SDL_WINDOWEVENT_CLOSE) return false;
                break;
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
