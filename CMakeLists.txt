cmake_minimum_required(VERSION 3.16)
project(sdlw LANGUAGES C CXX)

# C++20 for designated initializers ({.title = ...}) used by the window API.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# libSDL is the only external dependency. By default we build the vendored
# copy in vendor/SDL (a git submodule) so the project has zero system deps.
# Set SDLW_USE_SYSTEM_SDL=ON to link a system-installed SDL3 instead.
option(SDLW_USE_SYSTEM_SDL "Use system SDL3 instead of the vendored submodule" OFF)

if(SDLW_USE_SYSTEM_SDL)
    find_package(SDL3 REQUIRED)
else()
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL/CMakeLists.txt")
        message(FATAL_ERROR
            "vendor/SDL is empty. Fetch the submodule first:\n"
            "  git submodule update --init --recursive")
    endif()
    # Build SDL3 as a static lib in-tree; no install, no shared object.
    set(SDL_SHARED       OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC       ON  CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)
endif()

# Pick the SDL3 target to link (can't ALIAS an ALIAS, so use a variable).
if(TARGET SDL3::SDL3-static)
    set(SDLW_SDL_TARGET SDL3::SDL3-static)
elseif(TARGET SDL3::SDL3)
    set(SDLW_SDL_TARGET SDL3::SDL3)
else()
    message(FATAL_ERROR "No SDL3 target found")
endif()

# The sdlw framework: window backend + platform entry point.
add_library(sdlw STATIC
    src/window.cpp
    src/entry.cpp
    src/font.cpp
    src/button.cpp
    src/textbox.cpp
    src/listbox.cpp
    src/combobox.cpp
    src/label.cpp
    src/progressbar.cpp
    src/checkbox.cpp
    src/radiogroup.cpp
    src/select.cpp
)
target_include_directories(sdlw PUBLIC include)
target_link_libraries(sdlw PUBLIC ${SDLW_SDL_TARGET})

# --- Asset embedding -------------------------------------------------------
# Turn a binary asset into a compiled C byte array (regenerated when it
# changes). Fully in-house: just CMake + the C compiler, no extra tools.
set(SDLW_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${SDLW_GEN_DIR})

function(sdlw_embed_asset input symbol out_var)
    set(out "${SDLW_GEN_DIR}/${symbol}.c")
    add_custom_command(
        OUTPUT  "${out}"
        COMMAND ${CMAKE_COMMAND}
                -DINPUT=${input} -DOUTPUT=${out} -DSYMBOL=${symbol}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/tools/bin2c.cmake
        DEPENDS ${input} ${CMAKE_CURRENT_SOURCE_DIR}/tools/bin2c.cmake
        COMMENT "Embedding ${input}"
        VERBATIM)
    set(${out_var} "${out}" PARENT_SCOPE)
endfunction()

# Bake every DejaVu Sans size into the executable.
set(SDLW_FONT_SIZES 14 16 20 24 32)
set(SDLW_EMBEDDED_ASSETS "")
foreach(sz IN LISTS SDLW_FONT_SIZES)
    sdlw_embed_asset(${CMAKE_CURRENT_SOURCE_DIR}/assets/dejavusans_${sz}.fnt dejavusans_${sz}_fnt FNT_C)
    sdlw_embed_asset(${CMAKE_CURRENT_SOURCE_DIR}/assets/dejavusans_${sz}.bmp dejavusans_${sz}_bmp BMP_C)
    list(APPEND SDLW_EMBEDDED_ASSETS ${FNT_C} ${BMP_C})
endforeach()

# Example GUI app. On Windows, WIN32 makes it a GUI-subsystem exe (uses WinMain).
# The generated .c files bake the font atlases + descriptors into the executable.
add_executable(sdlw_example WIN32 example/main.cpp ${SDLW_EMBEDDED_ASSETS})
target_link_libraries(sdlw_example PRIVATE sdlw)

# --- Tests (in-house harness, no external deps) ----------------------------
# Pure-logic unit tests over the shared internal headers in src/. They don't
# link SDL, so they build fast. Run with:  ctest --test-dir build
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    option(SDLW_BUILD_TESTS "Build sdlw unit tests" ON)
endif()
if(SDLW_BUILD_TESTS)
    enable_testing()

    # Pure-logic tests: header-only, no SDL headers needed.
    foreach(t test_text test_bmfont)
        add_executable(${t} tests/${t}.cpp)
        target_include_directories(${t} PRIVATE src tests)
        add_test(NAME ${t} COMMAND ${t})
    endforeach()

    # Headless widget tests: compile the widget sources against SDL *headers*
    # plus a no-op SDL link stub, so they run without a display or the SDL
    # library. Widget update() logic never calls SDL at runtime (input comes
    # from the headless Window), so the stubs are only there to satisfy linking.
    add_executable(test_widgets
        tests/test_widgets.cpp
        tests/sdl_stub.cpp
        src/window.cpp src/font.cpp
        src/button.cpp src/textbox.cpp src/listbox.cpp src/combobox.cpp
        src/checkbox.cpp src/radiogroup.cpp src/select.cpp)
    target_include_directories(test_widgets PRIVATE
        include src tests ${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL/include)
    add_test(NAME test_widgets COMMAND test_widgets)
endif()
