// Unit tests for sdlw's BMFont descriptor parsing (src/bmfont.h).
#include "check.h"
#include "bmfont.h"

using namespace sdlw::detail;

TEST(bmf_get_int_key_boundaries) {
    std::string line =
        "char id=72 x=224 y=16 width=11 height=10 xoffset=0 yoffset=3 xadvance=11 page=0 chnl=15";
    int v = -1;
    CHECK(bmfGetInt(line, "id", v));      CHECK_EQ(v, 72);
    CHECK(bmfGetInt(line, "x", v));       CHECK_EQ(v, 224);   // must not match "xoffset"/"xadvance"
    CHECK(bmfGetInt(line, "y", v));       CHECK_EQ(v, 16);
    CHECK(bmfGetInt(line, "width", v));   CHECK_EQ(v, 11);
    CHECK(bmfGetInt(line, "xoffset", v)); CHECK_EQ(v, 0);
    CHECK(bmfGetInt(line, "yoffset", v)); CHECK_EQ(v, 3);
    CHECK(bmfGetInt(line, "xadvance", v));CHECK_EQ(v, 11);
}

TEST(bmf_get_int_negative_and_missing) {
    std::string line = "char id=74 xoffset=-1 yoffset=3 xadvance=4";
    int v = 999;
    CHECK(bmfGetInt(line, "xoffset", v)); CHECK_EQ(v, -1);   // handles negative
    v = 999;
    CHECK(!bmfGetInt(line, "height", v)); CHECK_EQ(v, 999);  // missing key: unchanged, false
}

TEST(bmf_get_str_quoted) {
    std::string line = "page id=0 file=\"dejavusans_14.bmp\"";
    CHECK_STR_EQ(bmfGetStr(line, "file"), "dejavusans_14.bmp");
    CHECK_STR_EQ(bmfGetStr(line, "missing"), ""); // absent key -> empty
}

SDLW_TEST_MAIN()
