// Internal text utilities shared by sdlw widgets (and unit-tested directly).
// Pure, header-only, no SDL dependency. Not part of the public API.
#pragma once

#include <cctype>
#include <cstddef>
#include <string>
#include <utility>

namespace sdlw::detail {

// --- UTF-8 caret movement over a byte buffer -------------------------------
inline std::size_t prevCharStart(const std::string& s, std::size_t i) {
    if (i == 0) return 0;
    --i;
    while (i > 0 && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80) --i;
    return i;
}
inline std::size_t nextCharStart(const std::string& s, std::size_t i) {
    std::size_t n = s.size();
    if (i >= n) return n;
    ++i;
    while (i < n && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80) ++i;
    return i;
}

// --- Word navigation -------------------------------------------------------
// Character classes: 0 = whitespace, 1 = word (alphanumeric, underscore, or any
// non-ASCII byte), 2 = punctuation.
inline int charClass(unsigned char c) {
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') return 0;
    if (c >= 0x80 || c == '_' || std::isalnum(c)) return 1;
    return 2;
}
inline bool isSpace(const std::string& s, std::size_t i) {
    return charClass(static_cast<unsigned char>(s[i])) == 0;
}
// Word jump stops at the next space/graphic boundary, moving over exactly one
// run: from a graphic char, stop before the first space; from a space, stop
// before the first graphic char.
inline std::size_t wordLeft(const std::string& s, std::size_t i) {
    if (i == 0) return 0;
    bool sp = isSpace(s, i - 1);
    while (i > 0 && isSpace(s, i - 1) == sp) --i;
    return i;
}
inline std::size_t wordRight(const std::string& s, std::size_t i) {
    std::size_t n = s.size();
    if (i >= n) return n;
    bool sp = isSpace(s, i);
    while (i < n && isSpace(s, i) == sp) ++i;
    return i;
}
// Bounds of the run of same-class characters around byte index i (double-click).
inline std::pair<std::size_t, std::size_t> wordBounds(const std::string& s, std::size_t i) {
    std::size_t n = s.size();
    if (n == 0) return { 0, 0 };
    std::size_t p = (i >= n) ? n - 1 : i;
    int cls = charClass(static_cast<unsigned char>(s[p]));
    std::size_t lo = p, hi = p + 1;
    while (lo > 0 && charClass(static_cast<unsigned char>(s[lo - 1])) == cls) --lo;
    while (hi < n && charClass(static_cast<unsigned char>(s[hi])) == cls) ++hi;
    return { lo, hi };
}

// --- Case-insensitive substring match (for combo filtering) ----------------
inline std::string toLower(std::string s) {
    for (char& c : s) c = char(std::tolower(static_cast<unsigned char>(c)));
    return s;
}
inline bool containsCI(const std::string& hay, const std::string& needle) {
    if (needle.empty()) return true;
    return toLower(hay).find(toLower(needle)) != std::string::npos;
}

} // namespace sdlw::detail
