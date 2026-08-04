// No-op SDL link stub for headless widget tests.
//
// Widget update() logic and Font::loadMetrics() never call SDL at runtime
// (input is injected via the headless Window). These stubs exist only so the
// test executable links without pulling in the real SDL library — they are
// never executed. Definitions inherit C linkage from the SDL3 headers.
#include <SDL3/SDL.h>

extern "C" {

bool          SDL_Init(SDL_InitFlags) { return true; }
void          SDL_Quit(void) {}
void          SDL_SetMainReady(void) {}
const char*   SDL_GetError(void) { return ""; }
void          SDL_free(void*) {}

SDL_Window*   SDL_CreateWindow(const char*, int, int, SDL_WindowFlags) { return nullptr; }
void          SDL_DestroyWindow(SDL_Window*) {}
bool          SDL_GetWindowSize(SDL_Window*, int* w, int* h) { if (w) *w = 0; if (h) *h = 0; return true; }
SDL_Renderer* SDL_CreateRenderer(SDL_Window*, const char*) { return nullptr; }
void          SDL_DestroyRenderer(SDL_Renderer*) {}
bool          SDL_SetRenderVSync(SDL_Renderer*, int) { return true; }

bool          SDL_PollEvent(SDL_Event*) { return false; }
SDL_MouseButtonFlags SDL_GetMouseState(float* x, float* y) { if (x) *x = 0; if (y) *y = 0; return 0; }
SDL_Keymod    SDL_GetModState(void) { return 0; }
Uint64        SDL_GetTicks(void) { return 0; }
bool          SDL_StartTextInput(SDL_Window*) { return true; }
bool          SDL_StopTextInput(SDL_Window*) { return true; }

char*         SDL_GetClipboardText(void) { return nullptr; }
bool          SDL_SetClipboardText(const char*) { return true; }

SDL_IOStream* SDL_IOFromFile(const char*, const char*) { return nullptr; }
SDL_IOStream* SDL_IOFromConstMem(const void*, size_t) { return nullptr; }
Sint64        SDL_GetIOSize(SDL_IOStream*) { return 0; }
size_t        SDL_ReadIO(SDL_IOStream*, void*, size_t) { return 0; }
bool          SDL_CloseIO(SDL_IOStream*) { return true; }

SDL_Surface*  SDL_LoadBMP(const char*) { return nullptr; }
SDL_Surface*  SDL_LoadBMP_IO(SDL_IOStream*, bool) { return nullptr; }
SDL_Surface*  SDL_ConvertSurface(SDL_Surface*, SDL_PixelFormat) { return nullptr; }
void          SDL_DestroySurface(SDL_Surface*) {}

SDL_Texture*  SDL_CreateTexture(SDL_Renderer*, SDL_PixelFormat, SDL_TextureAccess, int, int) { return nullptr; }
void          SDL_DestroyTexture(SDL_Texture*) {}
bool          SDL_UpdateTexture(SDL_Texture*, const SDL_Rect*, const void*, int) { return true; }
bool          SDL_SetTextureBlendMode(SDL_Texture*, SDL_BlendMode) { return true; }
bool          SDL_SetTextureColorMod(SDL_Texture*, Uint8, Uint8, Uint8) { return true; }
bool          SDL_SetTextureAlphaMod(SDL_Texture*, Uint8) { return true; }

bool          SDL_SetRenderDrawColor(SDL_Renderer*, Uint8, Uint8, Uint8, Uint8) { return true; }
bool          SDL_RenderClear(SDL_Renderer*) { return true; }
bool          SDL_RenderPresent(SDL_Renderer*) { return true; }
bool          SDL_RenderFillRect(SDL_Renderer*, const SDL_FRect*) { return true; }
bool          SDL_RenderRect(SDL_Renderer*, const SDL_FRect*) { return true; }
bool          SDL_RenderLine(SDL_Renderer*, float, float, float, float) { return true; }
bool          SDL_RenderTexture(SDL_Renderer*, SDL_Texture*, const SDL_FRect*, const SDL_FRect*) { return true; }
bool          SDL_SetRenderClipRect(SDL_Renderer*, const SDL_Rect*) { return true; }

} // extern "C"
