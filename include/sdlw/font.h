// sdlw::Font - renders text from a pre-baked bitmap-font atlas.
//
// The atlas (.bmp) and its BMFont descriptor (.fnt) are produced offline by
// tools/bake_font.py. At runtime this class only uses SDL: it loads the BMP,
// turns it into an alpha texture, and blits glyph rects. No TTF parsing here.
#pragma once

#include <string>

struct SDL_Renderer;

namespace sdlw {

class Font {
public:
    Font();
    ~Font();

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    // Load a baked font: pass the path to the .fnt descriptor. The atlas BMP
    // named in its `page` line is resolved relative to the .fnt's directory.
    // The resulting glyph texture is created on `renderer`, which is also used
    // for later draw() calls. Returns false on failure (see error()).
    bool load(SDL_Renderer* renderer, const std::string& fntPath);

    // Load from in-memory buffers instead of files — e.g. assets embedded in
    // the executable. `fnt` is the BMFont descriptor text; `bmp` is the raw
    // bytes of the atlas .bmp (the descriptor's page filename is ignored).
    bool loadFromMemory(SDL_Renderer* renderer,
                        const unsigned char* fnt, unsigned int fntLen,
                        const unsigned char* bmp, unsigned int bmpLen);

    // Parse only the descriptor metrics (no atlas texture, no renderer needed).
    // measure()/lineHeight()/base() work; draw() is a no-op. For tests/layout.
    bool loadMetrics(const unsigned char* fnt, unsigned int fntLen);

    bool ok() const;

    // Draw UTF-8 text with the top-left of the text block at (x, y).
    // '\n' begins a new line. Glyphs are tinted by (r, g, b).
    void draw(const std::string& utf8, float x, float y,
              unsigned char r = 255, unsigned char g = 255, unsigned char b = 255);

    // Pixel dimensions the text would occupy (handles '\n'). Either out may be null.
    void measure(const std::string& utf8, int* width, int* height) const;

    int lineHeight() const; // vertical distance between line tops, in pixels
    int base() const;       // ascent (line top to baseline), in pixels

    const char* error() const;

private:
    struct Impl;
    Impl* impl_;
};

} // namespace sdlw
