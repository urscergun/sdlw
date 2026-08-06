# Starter app using an installed sdlw package.
#
# Build:
#   cmake -B build -DCMAKE_PREFIX_PATH=/path/to/sdlw-install
#   cmake --build build
#
# Copy a baked font here first (or bake your own with bake_font.py):
#   cp /path/to/sdlw-install/share/sdlw/fonts/dejavusans_16.fnt assets/font_16.fnt
#   cp /path/to/sdlw-install/share/sdlw/fonts/dejavusans_16.bmp assets/font_16.bmp
cmake_minimum_required(VERSION 3.16)
project(myapp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the installed sdlw package (brings sdlw::sdlw + SDL3 + sdlw_embed_asset).
find_package(sdlw CONFIG REQUIRED)

# Bake the font into the executable.
sdlw_embed_asset(${CMAKE_CURRENT_SOURCE_DIR}/assets/font_16.fnt font_16_fnt FNT_C)
sdlw_embed_asset(${CMAKE_CURRENT_SOURCE_DIR}/assets/font_16.bmp font_16_bmp BMP_C)

# WIN32 makes it a GUI-subsystem app on Windows (uses the WinMain sdlw provides).
add_executable(myapp WIN32 src/main.cpp ${FNT_C} ${BMP_C})
target_link_libraries(myapp PRIVATE sdlw::sdlw)
