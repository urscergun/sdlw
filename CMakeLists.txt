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
)
target_include_directories(sdlw PUBLIC include)
target_link_libraries(sdlw PUBLIC ${SDLW_SDL_TARGET})

# Example GUI app. On Windows, WIN32 makes it a GUI-subsystem exe (uses WinMain).
add_executable(sdlw_example WIN32 example/main.cpp)
target_link_libraries(sdlw_example PRIVATE sdlw)
