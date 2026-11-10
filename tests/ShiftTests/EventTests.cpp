#include "TestFramework.h"

#include <algorithm>

#include "shift/BoardEvent.h"
#include "shift/Random.h"

using namespace shift;

namespace {
Board randomBoard(std::uint32_t seed) {
    Random rng(seed);
    Board b;
    for (int i = 0; i < kCells; ++i) b.set(fromIndex(i), static_cast<Cell>(rng.below(3)));
    return b;
}
}  // namespace

TEST(Event_SlideRowRightWraps) {
    Board b;
    b.set({0, 0}, Cell::Black);
    b.set({9, 0}, Cell::White);
    const Board a = BoardEvent{EventKind::SlideRow, 0, EventDir::Positive}.applied(b);
    CHECK(a.at({1, 0}) == Cell::Black);
    CHECK(a.at({0, 0}) == Cell::White);  // J1 → A1 へ回り込み
    CHECK(a.count(Cell::Black) == 1 && a.count(Cell::White) == 1);
}

TEST(Event_SlideRowLeftWraps) {
    Board b;
    b.set({0, 3}, Cell::Black);
    b.set({9, 3}, Cell::White);
    const Board a = BoardEvent{EventKind::SlideRow, 3, EventDir::Negative}.applied(b);
    CHECK(a.at({9, 3}) == Cell::Black);  // A4 → J4
    CHECK(a.at({8, 3}) == Cell::White);
}

TEST(Event_SlideColDownAndUpWrap) {
    Board b;
    b.set({2, 9}, Cell::Black);
    b.set({2, 0}, Cell::White);
    const Board down = BoardEvent{EventKind::SlideCol, 2, EventDir::Positive}.applied(b);
    CHECK(down.at({2, 0}) == Cell::Black);  // C10 → C1
    CHECK(down.at({2, 1}) == Cell::White);
    const Board up = BoardEvent{EventKind::SlideCol, 2, EventDir::Negative}.applied(b);
    CHECK(up.at({2, 9}) == Cell::White);  // C1 → C10
    CHECK(up.at({2, 8}) == Cell::Black);
}

TEST(Event_SlideOnlyAffectsTargetLine) {
    const Board b = randomBoard(7);
    const BoardEvent e{EventKind::SlideRow, 4, EventDir::Positive};
    const Board a = e.applied(b);
    for (int y = 0; y < kSize; ++y) {
        if (y == 4) continue;
        for (int x = 0; x < kSize; ++x) CHECK(a.at({x, y}) == b.at({x, y}));
    }
}

TEST(Event_RotateTopLeftClockwise) {
    Board b;
    b.set({0, 0}, Cell::Black);
    b.set({4, 0}, Cell::White);
    const Board a = BoardEvent{EventKind::Rotate, 0, EventDir::Positive}.applied(b);
    CHECK(a.at({4, 0}) == Cell::Black);  // 左上隅 → 右上隅
    CHECK(a.at({4, 4}) == Cell::White);  // 右上隅 → 右下隅
    CHECK(a.at({0, 0}) == Cell::Empty);
}

TEST(Event_RotateCounterClockwiseAndOtherQuadrants) {
    Board b;
    b.set({0, 0}, Cell::Black);
    const Board ccw = BoardEvent{EventKind::Rotate, 0, EventDir::Negative}.applied(b);
    CHECK(ccw.at({0, 4}) == Cell::Black);  // 左上隅 → 左下隅

    Board c;
    c.set({5, 5}, Cell::White);  // 右下区画の左上隅
    const Board cw = BoardEvent{EventKind::Rotate, 3, EventDir::Positive}.applied(c);
    CHECK(cw.at({9, 5}) == Cell::White);

    Board d;
    d.set({5, 0}, Cell::Black);  // 右上区画
    d.set({0, 5}, Cell::White);  // 左下区画
    const Board tr = BoardEvent{EventKind::Rotate, 1, EventDir::Positive}.applied(d);
    CHECK(tr.at({9, 0}) == Cell::Black);
    CHECK(tr.at({0, 5}) == Cell::White);  // 他区画は不変
    const Board bl = BoardEvent{EventKind::Rotate, 2, EventDir::Negative}.applied(d);
    CHECK(bl.at({0, 9}) == Cell::White);
    CHECK(bl.at({5, 0}) == Cell::Black);
}

TEST(Event_RotationInverseAndPeriod) {
    const Board b = randomBoard(11);
    for (int q = 0; q < kQuadrantCount; ++q) {
        const BoardEvent cw{EventKind::Rotate, q, EventDir::Positive};
        const BoardEvent ccw{EventKind::Rotate, q, EventDir::Negative};
        CHECK(ccw.applied(cw.applied(b)) == b);
        CHECK(cw.applied(cw.applied(cw.applied(cw.applied(b)))) == b);
    }
}

TEST(Event_CountsPreservedAndMapPosConsistent) {
    for (std::uint32_t seed = 1; seed <= 5; ++seed) {
        const Board b = randomBoard(seed);
        for (const BoardEvent& e : BoardEvent::allCandidates()) {
            const Board a = e.applied(b);
            CHECK(a.count(Cell::Black) == b.count(Cell::Black));
            CHECK(a.count(Cell::White) == b.count(Cell::White));
            for (int i = 0; i < kCells; ++i) {
                const Pos p = fromIndex(i);
                CHECK(a.at(e.mapPos(p)) == b.at(p));
            }
        }
    }
}

TEST(Event_AllCandidatesAre48Unique) {
    const auto all = BoardEvent::allCandidates();
    CHECK(all.size() == 48);
    const auto rotations = std::count_if(all.begin(), all.end(), [](const BoardEvent& e) { return e.kind == EventKind::Rotate; });
    CHECK(rotations == 8);
    for (size_t i = 0; i < all.size(); ++i)
        for (size_t j = i + 1; j < all.size(); ++j) CHECK(!(all[i] == all[j]));
}

TEST(Event_Describe) {
    CHECK(BoardEvent{EventKind::SlideRow, 2, EventDir::Positive}.describe() == "3行目を→ 右へ1マススライド");
    CHECK(BoardEvent{EventKind::SlideCol, 0, EventDir::Negative}.describe() == "A列を↑ 上へ1マススライド");
    CHECK(BoardEvent{EventKind::Rotate, 0, EventDir::Positive}.describe() == "左上区画を時計回りに90°回転");
    CHECK(BoardEvent{EventKind::Rotate, 3, EventDir::Negative}.describe() == "右下区画を反時計回りに90°回転");
}
