# Using sdlw in your own app

sdlw ships as a CMake package: a static library plus public headers and a
`find_package` config. Build it once into an install prefix, then link it from
your app.

## 1. Build and install the SDK

```sh
git clone <sdlw-repo> && cd sdlw
git submodule update --init --recursive     # fetches the vendored SDL3
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cmake --install build --prefix ./sdlw-install
```

This produces a self-contained SDK under `sdlw-install/`:

```
include/sdlw/*.h            public headers (window, font, widgets, layout, focus)
lib/libsdlw.a              the sdlw static library
lib/libSDL3.a              the vendored SDL3 static library
lib/cmake/sdlw/            find_package(sdlw) config + sdlw_embed_asset helper
lib/cmake/SDL3/            SDL3's package config (resolved automatically)
share/sdlw/fonts/          baked DejaVu Sans atlases (.fnt + .bmp)
share/sdlw/tools/          bake_font.py to bake your own fonts
```

## 2. Consume it from your CMake project

```cmake
cmake_minimum_required(VERSION 3.16)
project(myapp LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(sdlw CONFIG REQUIRED)          # sdlw::sdlw + SDL3 + sdlw_embed_asset

# Bake a font into the exe (copy one from share/sdlw/fonts, or bake your own).
sdlw_embed_asset(${CMAKE_SOURCE_DIR}/assets/font_16.fnt font_16_fnt FNT_C)
sdlw_embed_asset(${CMAKE_SOURCE_DIR}/assets/font_16.bmp font_16_bmp BMP_C)

add_executable(myapp WIN32 src/main.cpp ${FNT_C} ${BMP_C})
target_link_libraries(myapp PRIVATE sdlw::sdlw)
```

Configure with the install prefix on the search path:

```sh
cmake -B build -DCMAKE_PREFIX_PATH=/abs/path/to/sdlw-install
cmake --build build
```

A ready-to-copy starter is in `template/` (see `template/src/main.cpp`).

## 3. Write your app

You implement `int Main(int argc, char** argv)` — sdlw provides the real entry
point (`WinMain` on Windows, `main` on Linux) inside the library, so on Windows
build a GUI-subsystem executable (`add_executable(app WIN32 ...)`).

```cpp
#include "sdlw/window.h"
#include "sdlw/font.h"
#include "sdlw/button.h"

extern "C" { extern const unsigned char font_16_fnt[]; extern const unsigned int font_16_fnt_len;
             extern const unsigned char font_16_bmp[]; extern const unsigned int font_16_bmp_len; }

int Main(int, char**) {
    sdlw::Window win({ .title = "hi", .width = 400, .height = 200 });
    sdlw::Font ui;
    ui.loadFromMemory(win.renderer(), font_16_fnt, font_16_fnt_len, font_16_bmp, font_16_bmp_len);
    sdlw::Button ok("Click me", 20, 20, 120, 36);
    while (win.pumpEvents()) {
        ok.update(win);
        win.clear(24, 24, 32);
        ok.draw(win.renderer(), ui);
        win.present();
    }
    return 0;
}
```

## Baking your own font

```sh
python3 share/sdlw/tools/bake_font.py MyFont.ttf --size 16 --out assets/font_16
```

This emits `assets/font_16.fnt` + `assets/font_16.bmp`; embed them with
`sdlw_embed_asset` as above.

## Notes

- The install bundles the vendored SDL3 (static). `find_package(sdlw)` pulls in
  SDL3 automatically; you don't call `find_package(SDL3)` yourself.
- To link a system SDL3 instead of the vendored one, configure sdlw with
  `-DSDLW_USE_SYSTEM_SDL=ON` before installing.
- `cpack` from the sdlw build dir produces a `sdlw-<version>.tar.gz` archive of
  the install tree.
