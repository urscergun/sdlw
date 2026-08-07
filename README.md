# sdlw

A small, in-house C++20 GUI toolkit built on **libSDL3** — SDL is the only
external dependency; everything else (widgets, text rendering, layout, focus) is
hand-written. SDL3 is vendored as a git submodule and built statically, so the
project has no system dependencies to install.

Widgets: Window, Font (bitmap atlas), Label, Button, Checkbox, RadioGroup,
ProgressBar, TextBox, ListBox, ComboBox, Select, ListView (sortable table),
Scrollbar, ScrollView, plus a FocusManager (Tab traversal) and a layout engine
(VBox/HBox/Grid).

## Prerequisites

- CMake ≥ 3.16
- A C++20 compiler (GCC, Clang, or MSVC)
- git

## Quick build

```sh
git clone --recurse-submodules https://github.com/urscergun/sdlw.git
cd sdlw
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

If you cloned without `--recurse-submodules`, fetch SDL3 first:

```sh
git submodule update --init --recursive
```

The first build also compiles the vendored SDL3, so it takes a while; later
builds are incremental.

Run the demo (a window showing every widget):

```sh
./build/sdlw_example              # Linux
build\Release\sdlw_example.exe    # Windows (MSVC)
```

## Windows

MSVC is a multi-config generator — pick the config at build time:

```sh
cmake -B build
cmake --build build --config Release
```

The example is a GUI-subsystem app (`WIN32`), so it launches without a console.

## Tests

```sh
ctest --test-dir build --output-on-failure
```

## Use sdlw in your own app

Install the SDK (headers + `libsdlw.a` + SDL3 + a `find_package` config):

```sh
cmake --install build --prefix ./sdlw-install
```

Then in your project:

```cmake
find_package(sdlw CONFIG REQUIRED)          # -DCMAKE_PREFIX_PATH=.../sdlw-install
target_link_libraries(myapp PRIVATE sdlw::sdlw)
```

See [docs/USING.md](docs/USING.md) for the full guide (embedding fonts, the
starter template in `template/`, baking your own font).

## Layout & options

- `-DSDLW_USE_SYSTEM_SDL=ON` — link a system SDL3 instead of the vendored one.
- `-DSDLW_BUILD_TESTS=OFF` — skip the unit tests.
- Smaller Linux release binary: add
  `-DCMAKE_CXX_FLAGS="-ffunction-sections -fdata-sections"` and
  `-DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections -s"`.
