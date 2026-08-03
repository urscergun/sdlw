// Internal BMFont ('.fnt') descriptor parsing helpers, shared by the font
// loader and unit-tested directly. Pure, header-only, no SDL dependency.
#pragma once

#include <cstdlib>
#include <string>

namespace sdlw::detail {

// Parse " key=<int>" out of a BMFont line. Prepending a space makes each token
// space-delimited, so "x=" cannot match inside "xoffset=" / "xadvance=".
inline bool bmfGetInt(const std::string& line, const char* key, int& out) {
    std::string needle = std::string(" ") + key + "=";
    std::string hay = std::string(" ") + line;
    auto p = hay.find(needle);
    if (p == std::string::npos) return false;
    out = std::atoi(hay.c_str() + p + needle.size()); // stops at space; handles '-'
    return true;
}

// Parse ' key="..."' (a quoted string value) out of a BMFont line.
inline std::string bmfGetStr(const std::string& line, const char* key) {
    std::string needle = std::string(" ") + key + "=\"";
    std::string hay = std::string(" ") + line;
    auto p = hay.find(needle);
    if (p == std::string::npos) return {};
    p += needle.size();
    auto e = hay.find('"', p);
    if (e == std::string::npos) return {};
    return hay.substr(p, e - p);
}

} // namespace sdlw::detail
