#include "TestFramework.h"

#include <algorithm>

using namespace shift;
using testfw::parseBoard;

namespace {
bool contains(const std::vector<Pos>& v, Pos p) { return std::find(v.begin(), v.end(), p) != v.end(); }
}  // namespace

TEST(Board_InitialLayout) {
    const Board b = Board::initial();
    CHECK(b.at({4, 4}) == Cell::White);  // E5
    CHECK(b.at({5, 4}) == Cell::Black);  // F5
    CHECK(b.at({4, 5}) == Cell::Black);  // E6
    CHECK(b.at({5, 5}) == Cell::White);  // F6
    CHECK(b.count(Cell::Black) == 2);
    CHECK(b.count(Cell::White) == 2);
    CHECK(b.emptyCount() == 96);
    CHECK(posName({4, 4}) == "E5");
    CHECK(posName({9, 9}) == "J10");
}

TEST(Board_InitialLegalMovesForBlack) {
    const Board b = Board::initial();
    const auto moves = b.legalMoves(Cell::Black);
    CHECK(moves.size() == 4);
    CHECK(contains(moves, {4, 3}));  // E4
    CHECK(contains(moves, {3, 4}));  // D5
    CHECK(contains(moves, {6, 5}));  // G6
    CHECK(contains(moves, {5, 6}));  // F7
}

TEST(Board_FlipsInMultipleDirections) {
    Board b = parseBoard({
        "B.B.B.....",
        ".WWW......",
        "BW.WB.....",
        ".WWW......",
        "B.B.B.....",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
    });
    const auto flips = b.place({2, 2}, Cell::Black);
    CHECK(flips.size() == 8);
    for (int y = 1; y <= 3; ++y)
        for (int x = 1; x <= 3; ++x) CHECK(b.at({x, y}) == Cell::Black);
}

TEST(Board_NoWrapAroundForFlips) {
    // 行の端で挟み判定は打ち切る（回り込みなし）
    const Board b = parseBoard({
        "WB.......B",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
    });
    // C1 に白: 左に B1(黒) A1(白) → 合法
    CHECK(b.isLegal({2, 0}, Cell::White));
    // I1 に白: 右に J1(黒) その先は盤外 → A1 へ回り込まないので非合法
    CHECK(!b.isLegal({8, 0}, Cell::White));
}

TEST(Board_IllegalMoves) {
    Board b = Board::initial();
    CHECK(!b.isLegal({0, 0}, Cell::Black));  // 何も挟めない
    CHECK(!b.isLegal({4, 4}, Cell::Black));  // 既に石がある
    const Board before = b;
    CHECK(b.place({0, 0}, Cell::Black).empty());
    CHECK(b == before);  // 非合法手は盤を変えない
}

TEST(Board_FullDetection) {
    Board b;
    for (int i = 0; i < kCells; ++i) b.set(fromIndex(i), Cell::White);
    CHECK(b.isFull());
    b.set({3, 3}, Cell::Empty);
    CHECK(!b.isFull());
    CHECK(b.emptyCount() == 1);
}
