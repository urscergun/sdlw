cmake_minimum_required(VERSION 3.16)
project(sdlw VERSION 0.1.0 LANGUAGES C CXX)

# C++20 for designated initializers ({.title = ...}) used by the window API.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

# Whether to generate install/package rules (headers + lib + CMake config).
# On by default for a top-level build; off when sdlw is a subproject.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(_sdlw_top_level ON)
else()
    set(_sdlw_top_level OFF)
endif()
option(SDLW_INSTALL "Generate install/package rules for sdlw" ${_sdlw_top_level})

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
    # Build SDL3 as a static lib in-tree; no shared object.
    set(SDL_SHARED       OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC       ON  CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    # When we install sdlw, also install the vendored SDL3 so apps that link the
    # installed libsdlw can resolve SDL3 (find_dependency(SDL3) in the config).
    if(SDLW_INSTALL)
        set(SDL_INSTALL ON CACHE BOOL "" FORCE)
    endif()
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
    src/focus.cpp
    src/scrollbar.cpp
    src/scrollview.cpp
    src/listview.cpp
    src/layout.cpp
)
target_include_directories(sdlw PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(sdlw PUBLIC ${SDLW_SDL_TARGET})
add_library(sdlw::sdlw ALIAS sdlw)

# --- Asset embedding -------------------------------------------------------
# sdlw_embed_asset(): bake a binary asset (e.g. a baked font) into a compiled
# C byte array. Shared with consumers via the installed package.
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdlwEmbed.cmake)

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

    # Pure-logic tests: no SDL. test_layout also uses the public layout header.
    foreach(t test_text test_bmfont)
        add_executable(${t} tests/${t}.cpp)
        target_include_directories(${t} PRIVATE src tests)
        add_test(NAME ${t} COMMAND ${t})
    endforeach()
    add_executable(test_layout tests/test_layout.cpp src/layout.cpp)
    target_include_directories(test_layout PRIVATE include tests)
    add_test(NAME test_layout COMMAND test_layout)

    # Headless widget tests: compile the widget sources against SDL *headers*
    # plus a no-op SDL link stub, so they run without a display or the SDL
    # library. Widget update() logic never calls SDL at runtime (input comes
    # from the headless Window), so the stubs are only there to satisfy linking.
    add_executable(test_widgets
        tests/test_widgets.cpp
        tests/sdl_stub.cpp
        src/window.cpp src/font.cpp
        src/button.cpp src/textbox.cpp src/listbox.cpp src/combobox.cpp
        src/checkbox.cpp src/radiogroup.cpp src/select.cpp src/focus.cpp
        src/scrollbar.cpp src/scrollview.cpp src/listview.cpp)
    target_include_directories(test_widgets PRIVATE
        include src tests ${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL/include)
    add_test(NAME test_widgets COMMAND test_widgets)
endif()

# --- Install / package -----------------------------------------------------
# `cmake --install <build> --prefix <dir>` produces a self-contained SDK:
#   include/sdlw/*.h         public headers
#   lib/libsdlw.a            the sdlw static library (+ SDL3 static lib)
#   lib/cmake/sdlw/*.cmake   package config so apps can find_package(sdlw)
#   share/sdlw/tools         bake_font.py to bake your own fonts
# Consuming app:
#   find_package(sdlw CONFIG REQUIRED)   # CMAKE_PREFIX_PATH -> the install dir
#   target_link_libraries(app PRIVATE sdlw::sdlw)
if(SDLW_INSTALL)
    include(CMakePackageConfigHelpers)

    install(TARGETS sdlw EXPORT sdlwTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    install(DIRECTORY include/sdlw DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(EXPORT sdlwTargets
        NAMESPACE sdlw::
        FILE sdlwTargets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sdlw)

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdlwConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/sdlwConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sdlw)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/sdlwConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/sdlwConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/sdlwConfigVersion.cmake
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdlwEmbed.cmake
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/bin2c.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sdlw)

    # Font-baking tool + baked DejaVu Sans atlases (bake or embed your own).
    install(PROGRAMS tools/bake_font.py DESTINATION ${CMAKE_INSTALL_DATADIR}/sdlw/tools)
    install(DIRECTORY assets/ DESTINATION ${CMAKE_INSTALL_DATADIR}/sdlw/fonts
        FILES_MATCHING PATTERN "dejavusans_*.fnt" PATTERN "dejavusans_*.bmp")

    # Optional: `cpack` from the build dir makes a distributable archive.
    set(CPACK_PACKAGE_NAME sdlw)
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_GENERATOR "TGZ")
    include(CPack)
endif()
