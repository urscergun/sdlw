#include "sdlw/font.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>

namespace sdlw {

namespace {

// Parse " key=<int>" out of a BMFont line. Prepending a space makes each
// token space-delimited so "x=" cannot match inside "xoffset=", etc.
bool getInt(const std::string& line, const char* key, int& out) {
    std::string needle = std::string(" ") + key + "=";
    std::string hay = std::string(" ") + line;
    auto p = hay.find(needle);
    if (p == std::string::npos) return false;
    out = std::atoi(hay.c_str() + p + needle.size()); // stops at space; handles '-'
    return true;
}

// Parse ' key="..."' (a quoted string value) out of a BMFont line.
std::string getStr(const std::string& line, const char* key) {
    std::string needle = std::string(" ") + key + "=\"";
    std::string hay = std::string(" ") + line;
    auto p = hay.find(needle);
    if (p == std::string::npos) return {};
    p += needle.size();
    auto e = hay.find('"', p);
    if (e == std::string::npos) return {};
    return hay.substr(p, e - p);
}

std::string dirOf(const std::string& path) {
    auto p = path.find_last_of("/\\");
    return p == std::string::npos ? std::string() : path.substr(0, p + 1);
}

// Decode one UTF-8 codepoint starting at index i; advances i past it.
uint32_t nextCodepoint(const std::string& s, size_t& i) {
    unsigned char c = static_cast<unsigned char>(s[i]);
    uint32_t cp;
    int extra;
    if (c < 0x80)      { cp = c;        extra = 0; }
    else if (c < 0xE0) { cp = c & 0x1F; extra = 1; }
    else if (c < 0xF0) { cp = c & 0x0F; extra = 2; }
    else               { cp = c & 0x07; extra = 3; }
    ++i;
    for (int k = 0; k < extra && i < s.size(); ++k, ++i) {
        cp = (cp << 6) | (static_cast<unsigned char>(s[i]) & 0x3F);
    }
    return cp;
}

} // namespace

struct Glyph {
    int x = 0, y = 0, w = 0, h = 0; // atlas rect
    int xoff = 0, yoff = 0;         // placement offset from pen / line top
    int xadv = 0;                   // horizontal advance
};

struct Font::Impl {
    SDL_Renderer* renderer = nullptr;
    SDL_Texture*  texture = nullptr;
    std::unordered_map<uint32_t, Glyph> glyphs;
    int lineHeight = 0;
    int base = 0;
    std::string error;

    const Glyph* find(uint32_t cp) const {
        auto it = glyphs.find(cp);
        return it == glyphs.end() ? nullptr : &it->second;
    }

    // Parse a BMFont descriptor into glyphs/metrics; returns the page filename.
    std::string parseDescriptor(const std::string& text) {
        std::string pageFile;
        size_t start = 0;
        while (start <= text.size()) {
            size_t nl = text.find('\n', start);
            std::string line = text.substr(start, nl == std::string::npos ? std::string::npos : nl - start);
            if (!line.empty() && line.back() == '\r') line.pop_back();
            start = (nl == std::string::npos) ? text.size() + 1 : nl + 1;

            if (line.rfind("common", 0) == 0) {
                getInt(line, "lineHeight", lineHeight);
                getInt(line, "base", base);
            } else if (line.rfind("page", 0) == 0) {
                pageFile = getStr(line, "file");
            } else if (line.rfind("char ", 0) == 0) {
                int id = -1;
                if (!getInt(line, "id", id)) continue;
                Glyph g;
                getInt(line, "x", g.x);           getInt(line, "y", g.y);
                getInt(line, "width", g.w);       getInt(line, "height", g.h);
                getInt(line, "xoffset", g.xoff);  getInt(line, "yoffset", g.yoff);
                getInt(line, "xadvance", g.xadv);
                glyphs[static_cast<uint32_t>(id)] = g;
            }
        }
        return pageFile;
    }

    // Turn a loaded atlas surface (white glyphs on black) into an alpha
    // texture; consumes (destroys) `bmp`. Sets error on failure.
    bool buildAtlas(SDL_Surface* bmp) {
        if (!bmp) { error = std::string("load atlas: ") + SDL_GetError(); return false; }
        SDL_Surface* rgba = SDL_ConvertSurface(bmp, SDL_PIXELFORMAT_RGBA32);
        SDL_DestroySurface(bmp);
        if (!rgba) { error = std::string("convert atlas: ") + SDL_GetError(); return false; }

        const int w = rgba->w, h = rgba->h;
        std::vector<unsigned char> pixels(static_cast<size_t>(w) * h * 4);
        for (int row = 0; row < h; ++row) {
            const unsigned char* src = static_cast<unsigned char*>(rgba->pixels) + row * rgba->pitch;
            unsigned char* dst = pixels.data() + static_cast<size_t>(row) * w * 4;
            for (int col = 0; col < w; ++col) {
                unsigned char coverage = src[col * 4]; // R channel (R==G==B here)
                dst[col * 4 + 0] = 255;
                dst[col * 4 + 1] = 255;
                dst[col * 4 + 2] = 255;
                dst[col * 4 + 3] = coverage;
            }
        }
        SDL_DestroySurface(rgba);

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                    SDL_TEXTUREACCESS_STATIC, w, h);
        if (!texture) { error = std::string("create texture: ") + SDL_GetError(); return false; }
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_UpdateTexture(texture, nullptr, pixels.data(), w * 4);
        return true;
    }
};

Font::Font() : impl_(new Impl) {}

Font::~Font() {
    if (impl_->texture) SDL_DestroyTexture(impl_->texture);
    delete impl_;
}

bool Font::load(SDL_Renderer* renderer, const std::string& fntPath) {
    impl_->renderer = renderer;

    SDL_IOStream* io = SDL_IOFromFile(fntPath.c_str(), "r");
    if (!io) { impl_->error = std::string("open .fnt: ") + SDL_GetError(); return false; }
    Sint64 sz = SDL_GetIOSize(io);
    std::string text;
    if (sz > 0) {
        text.resize(static_cast<size_t>(sz));
        SDL_ReadIO(io, text.data(), static_cast<size_t>(sz));
    }
    SDL_CloseIO(io);

    std::string pageFile = impl_->parseDescriptor(text);
    if (pageFile.empty()) { impl_->error = "descriptor has no page file"; return false; }

    std::string bmpPath = dirOf(fntPath) + pageFile;
    SDL_Surface* bmp = SDL_LoadBMP(bmpPath.c_str());
    if (!bmp) { impl_->error = std::string("load atlas '") + bmpPath + "': " + SDL_GetError(); return false; }
    return impl_->buildAtlas(bmp);
}

bool Font::loadFromMemory(SDL_Renderer* renderer,
                          const unsigned char* fnt, unsigned int fntLen,
                          const unsigned char* bmp, unsigned int bmpLen) {
    impl_->renderer = renderer;

    std::string text(reinterpret_cast<const char*>(fnt), fntLen);
    impl_->parseDescriptor(text); // page filename is irrelevant in memory mode

    SDL_IOStream* io = SDL_IOFromConstMem(bmp, bmpLen);
    if (!io) { impl_->error = std::string("atlas mem io: ") + SDL_GetError(); return false; }
    SDL_Surface* surf = SDL_LoadBMP_IO(io, true /*closeio*/);
    return impl_->buildAtlas(surf);
}

bool Font::ok() const { return impl_->texture != nullptr; }

void Font::draw(const std::string& utf8, float x, float y,
                unsigned char r, unsigned char g, unsigned char b) {
    if (!impl_->texture) return;
    SDL_SetTextureColorMod(impl_->texture, r, g, b);
    SDL_SetTextureAlphaMod(impl_->texture, 255);

    float penX = x, penY = y;
    for (size_t i = 0; i < utf8.size();) {
        uint32_t cp = nextCodepoint(utf8, i);
        if (cp == '\n') { penX = x; penY += impl_->lineHeight; continue; }
        const Glyph* gl = impl_->find(cp);
        if (!gl) { if (const Glyph* sp = impl_->find(' ')) penX += sp->xadv; continue; }
        if (gl->w > 0 && gl->h > 0) {
            SDL_FRect src{ float(gl->x), float(gl->y), float(gl->w), float(gl->h) };
            SDL_FRect dst{ penX + gl->xoff, penY + gl->yoff, float(gl->w), float(gl->h) };
            SDL_RenderTexture(impl_->renderer, impl_->texture, &src, &dst);
        }
        penX += gl->xadv;
    }
}

void Font::measure(const std::string& utf8, int* width, int* height) const {
    int maxW = 0, lineW = 0, lines = 1;
    for (size_t i = 0; i < utf8.size();) {
        uint32_t cp = nextCodepoint(utf8, i);
        if (cp == '\n') { maxW = std::max(maxW, lineW); lineW = 0; ++lines; continue; }
        if (const Glyph* gl = impl_->find(cp)) lineW += gl->xadv;
    }
    maxW = std::max(maxW, lineW);
    if (width)  *width = maxW;
    if (height) *height = lines * impl_->lineHeight;
}

int Font::lineHeight() const { return impl_->lineHeight; }
int Font::base() const { return impl_->base; }
const char* Font::error() const { return impl_->error.c_str(); }

} // namespace sdlw
