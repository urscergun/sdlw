// Unit tests for sdlw's text utilities (src/text_util.h).
#include "check.h"
#include "text_util.h"

using namespace sdlw::detail;

TEST(utf8_caret_movement) {
    std::string u = "a\xC3\xA9\xE2\x82\xAC" "z"; // a é(2B) €(3B) z
    // forward across multi-byte chars
    std::size_t i = 0;
    i = nextCharStart(u, i); CHECK_EQ(i, std::size_t(1));  // past 'a'
    i = nextCharStart(u, i); CHECK_EQ(i, std::size_t(3));  // past é
    i = nextCharStart(u, i); CHECK_EQ(i, std::size_t(6));  // past €
    i = nextCharStart(u, i); CHECK_EQ(i, std::size_t(7));  // past 'z'
    i = nextCharStart(u, i); CHECK_EQ(i, u.size());        // clamp at end
    // backward
    i = prevCharStart(u, 6); CHECK_EQ(i, std::size_t(3));  // back over €
    i = prevCharStart(u, 3); CHECK_EQ(i, std::size_t(1));  // back over é
    CHECK_EQ(prevCharStart(u, 0), std::size_t(0));         // clamp at start
}

TEST(word_jump_space_graphic_boundary) {
    std::string s = "The quick  fox"; // two spaces between quick and fox
    // From a graphic char, stop before the first space.
    CHECK_EQ(wordRight(s, 0), std::size_t(3));   // end of "The"
    CHECK_EQ(wordRight(s, 4), std::size_t(9));   // end of "quick"
    // From a space, stop before the first graphic char (skip the whole run).
    CHECK_EQ(wordRight(s, 3), std::size_t(4));   // to start of "quick"
    CHECK_EQ(wordRight(s, 9), std::size_t(11));  // skip both spaces to "fox"
    // Left is symmetric.
    CHECK_EQ(wordLeft(s, s.size()), std::size_t(11)); // start of "fox"
    CHECK_EQ(wordLeft(s, 11), std::size_t(9));        // over the spaces
    CHECK_EQ(wordLeft(s, 9), std::size_t(4));         // start of "quick"
    // Punctuation counts as graphic.
    std::string p = "ab, cd";
    CHECK_EQ(wordRight(p, 0), std::size_t(3));   // "ab," then stop at space
}

TEST(double_click_word_bounds) {
    std::string s = "The quick brown_fox, jumps";
    auto q = wordBounds(s, 6);   // inside "quick"
    CHECK_STR_EQ(s.substr(q.first, q.second - q.first), "quick");
    auto b = wordBounds(s, 12);  // underscore joins into one word
    CHECK_STR_EQ(s.substr(b.first, b.second - b.first), "brown_fox");
    std::size_t comma = s.find(',');
    auto c = wordBounds(s, comma); // punctuation selects on its own
    CHECK_STR_EQ(s.substr(c.first, c.second - c.first), ",");
    std::string u = "caf\xC3\xA9 str"; // é stays intact
    auto w = wordBounds(u, 1);
    CHECK_STR_EQ(u.substr(w.first, w.second - w.first), "caf\xC3\xA9");
}

TEST(case_insensitive_filter) {
    CHECK(containsCI("Australia", "au"));
    CHECK(containsCI("Australia", "RA"));   // case-insensitive, mid-word
    CHECK(containsCI("Greece", "gr"));
    CHECK(!containsCI("Germany", "gr"));    // "gr" not in Germany
    CHECK(containsCI("anything", ""));      // empty needle matches
    CHECK(!containsCI("abc", "xyz"));
}

SDLW_TEST_MAIN()
