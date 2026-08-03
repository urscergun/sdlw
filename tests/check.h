// Minimal in-house test harness for sdlw — no external dependencies.
//
// Usage:
//     #include "check.h"
//     TEST(my_case) {
//         CHECK(1 + 1 == 2);
//         CHECK_EQ(foo(), 42);
//         CHECK_STR_EQ(bar(), "hi");
//     }
//     SDLW_TEST_MAIN()
//
// Each TEST auto-registers; SDLW_TEST_MAIN() runs them all and returns the
// number of failing checks (0 = success), so CTest reports pass/fail by exit
// code.
#pragma once

#include <cstdio>
#include <string>
#include <vector>

namespace sdlw_test {

struct Stats { int checks = 0; int failures = 0; };
inline Stats& stats() { static Stats s; return s; }

using TestFn = void (*)();
struct TestCase { const char* name; TestFn fn; };
inline std::vector<TestCase>& registry() { static std::vector<TestCase> r; return r; }

struct Registrar {
    Registrar(const char* name, TestFn fn) { registry().push_back({ name, fn }); }
};

inline void reportFail(const char* file, int line, const char* expr) {
    ++stats().failures;
    std::fprintf(stderr, "  FAIL %s:%d: %s\n", file, line, expr);
}

inline int runAll() {
    int failedCases = 0;
    for (const TestCase& t : registry()) {
        int before = stats().failures;
        t.fn();
        int failed = stats().failures - before;
        std::fprintf(stderr, "[%s] %s\n", failed ? "FAIL" : "ok  ", t.name);
        if (failed) ++failedCases;
    }
    std::fprintf(stderr, "\n%d checks, %d failures across %zu tests (%d failing)\n",
                 stats().checks, stats().failures, registry().size(), failedCases);
    return stats().failures;
}

} // namespace sdlw_test

#define TEST(name)                                                        \
    static void name();                                                   \
    static ::sdlw_test::Registrar sdlw_reg_##name(#name, name);           \
    static void name()

#define CHECK(cond)                                                       \
    do {                                                                  \
        ++::sdlw_test::stats().checks;                                     \
        if (!(cond)) ::sdlw_test::reportFail(__FILE__, __LINE__, "CHECK(" #cond ")"); \
    } while (0)

#define CHECK_EQ(a, b)                                                    \
    do {                                                                  \
        ++::sdlw_test::stats().checks;                                     \
        if (!((a) == (b)))                                                 \
            ::sdlw_test::reportFail(__FILE__, __LINE__, "CHECK_EQ(" #a ", " #b ")"); \
    } while (0)

// Compares std::string / const char* values by string content.
#define CHECK_STR_EQ(a, b)                                                \
    do {                                                                  \
        ++::sdlw_test::stats().checks;                                     \
        if (!(std::string(a) == std::string(b)))                          \
            ::sdlw_test::reportFail(__FILE__, __LINE__, "CHECK_STR_EQ(" #a ", " #b ")"); \
    } while (0)

#define SDLW_TEST_MAIN() int main() { return ::sdlw_test::runAll(); }
