#pragma once
// 外部依存なしの最小テストフレームワーク（憲章 II）。
#include <cstdio>
#include <string>
#include <vector>

#include "shift/Board.h"

namespace testfw {

struct TestCase {
    const char* name;
    void (*fn)();
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> r;
    return r;
}

inline int& failures() {
    static int f = 0;
    return f;
}

struct Registrar {
    Registrar(const char* name, void (*fn)()) { registry().push_back({name, fn}); }
};

// 10 行の文字列から盤面を作る。'.'=空, 'B'=黒, 'W'=白。
inline shift::Board parseBoard(const std::vector<std::string>& rows) {
    shift::Board b;
    for (int y = 0; y < shift::kSize; ++y) {
        for (int x = 0; x < shift::kSize; ++x) {
            const char c = rows[static_cast<size_t>(y)][static_cast<size_t>(x)];
            b.set({x, y}, c == 'B' ? shift::Cell::Black : c == 'W' ? shift::Cell::White : shift::Cell::Empty);
        }
    }
    return b;
}

}  // namespace testfw

#define TEST(name)                                                   \
    static void name();                                              \
    static ::testfw::Registrar registrar_##name(#name, &name);       \
    static void name()

// 可変引数にして、式中の波括弧初期化子（Pos{0, 0} など）のカンマを許容する
#define CHECK(...)                                                                      \
    do {                                                                                \
        if (!(__VA_ARGS__)) {                                                           \
            ++::testfw::failures();                                                     \
            std::printf("    FAIL %s(%d): %s\n", __FILE__, __LINE__, #__VA_ARGS__);     \
        }                                                                               \
    } while (0)
