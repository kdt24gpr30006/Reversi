#include "TestFramework.h"

#include "shift/Game.h"

using namespace shift;
using testfw::parseBoard;

namespace {
const BoardEvent kRow0Right{EventKind::SlideRow, 0, EventDir::Positive};

// 黒: 合法手なし / 白: C1 に合法手あり
Board mustPassBoard() {
    return parseBoard({
        "WB........",
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
}

// 両者合法手なし。2 行目を左へスライドすると C2 の白が B2 へ移り、黒が C3 に打てるようになる
Board bothBlockedBoard() {
    return parseBoard({
        "B.........",
        "..W.......",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
    });
}
}  // namespace

TEST(Game_InitialState) {
    const Game g(123);
    CHECK(g.turn() == Cell::Black);
    CHECK(g.counter() == 0);
    CHECK(g.movesUntilEvent() == 4);
    CHECK(g.status() == TurnStatus::NeedMove);
    CHECK(!(g.nextEvent().applied(g.board()) == g.board()));  // 予告は盤を変化させる
    CHECK(!g.lastMove().has_value());
}

TEST(Game_FourMovesTriggerEvent) {
    Game g(7);
    for (int i = 0; i < 4; ++i) {
        CHECK(g.status() == TurnStatus::NeedMove);
        const auto moves = g.legalMoves();
        const MoveResult r = g.play(moves.front());
        CHECK(!r.flipped.empty());
        CHECK(g.counter() == i + 1);
        CHECK(r.eventDue == (i == 3));
    }
    CHECK(g.status() == TurnStatus::EventDue);
    const BoardEvent expected = g.nextEvent();
    const Board before = g.board();
    const EventResult er = g.fireDueEvent();
    CHECK(er.event == expected);
    CHECK(!er.forced);
    CHECK(er.before == before);
    CHECK(g.board() == expected.applied(before));  // 移動のみ・反転なし（FR-004）
    CHECK(g.counter() == 0);
    CHECK(g.movesUntilEvent() == 4);
    CHECK(g.turn() == Cell::Black);  // 4 手後は黒番のまま
}

TEST(Game_EventDoesNotFlipSandwiches) {
    // イベント後に W を B で挟む形ができても反転しない
    // B 列を上へスライドすると B2 の白が B1 に入り、A1・C1 の黒に挟まれる
    Board b = parseBoard({
        "B.B.......",
        ".W........",
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "....WB....",
        "....BW....",
        "..........",
    });
    Game g(b, Cell::Black, 4, BoardEvent{EventKind::SlideCol, 1, EventDir::Negative}, 1);
    CHECK(g.status() == TurnStatus::EventDue);
    g.fireDueEvent();
    CHECK(g.board().at({0, 0}) == Cell::Black);
    CHECK(g.board().at({1, 0}) == Cell::White);
    CHECK(g.board().at({2, 0}) == Cell::Black);
    CHECK(g.board().count(Cell::White) == b.count(Cell::White));
}

TEST(Game_PassCountsAsMove) {
    Game g(mustPassBoard(), Cell::Black, 1, kRow0Right, 5);
    CHECK(g.status() == TurnStatus::MustPass);
    g.pass();
    CHECK(g.counter() == 2);
    CHECK(g.turn() == Cell::White);
    CHECK(g.status() == TurnStatus::NeedMove);
}

TEST(Game_PassReachingFourTriggersEvent) {
    Game g(mustPassBoard(), Cell::Black, 3, kRow0Right, 5);
    g.pass();
    CHECK(g.counter() == 4);
    CHECK(g.status() == TurnStatus::EventDue);
    g.fireDueEvent();
    CHECK(g.counter() == 0);
    CHECK(g.board().at({1, 0}) == Cell::White);
    CHECK(g.board().at({2, 0}) == Cell::Black);
}

TEST(Game_FullBoardEndsWithoutEvent) {
    Board b;
    for (int i = 0; i < kCells; ++i) b.set(fromIndex(i), Cell::White);
    b.set({0, 0}, Cell::Empty);
    b.set({2, 0}, Cell::Black);  // 黒 A1 で B1 を挟める
    Game g(b, Cell::Black, 3, kRow0Right, 9);
    const MoveResult r = g.play({0, 0});
    CHECK(r.gameOver);
    CHECK(!r.eventDue);
    CHECK(g.status() == TurnStatus::GameOver);
    CHECK(g.board().at({0, 0}) == Cell::Black);  // イベントは発動していない
}

TEST(Game_BothBlockedForcesEventWithoutCountingPass) {
    const BoardEvent row1Left{EventKind::SlideRow, 1, EventDir::Negative};
    Game g(bothBlockedBoard(), Cell::Black, 2, row1Left, 3);
    CHECK(g.status() == TurnStatus::BothBlocked);
    const EventResult er = g.resolveBothBlocked();
    CHECK(er.forced);
    CHECK(!er.gameOver);
    CHECK(er.event == row1Left);
    CHECK(g.counter() == 0);           // パスは加算されずリセット
    CHECK(g.turn() == Cell::Black);    // 手番はそのまま
    CHECK(g.status() == TurnStatus::NeedMove);
    CHECK(g.isLegal({2, 2}));          // C3
}

TEST(Game_BothBlockedStillBlockedEndsGame) {
    Board b;
    b.set({0, 0}, Cell::Black);  // 白石が無い → 何が起きても両者打てない
    Game g(b, Cell::White, 1, kRow0Right, 3);
    CHECK(g.status() == TurnStatus::BothBlocked);
    const EventResult er = g.resolveBothBlocked();
    CHECK(er.gameOver);
    CHECK(g.status() == TurnStatus::GameOver);
    CHECK(g.winner() == Cell::Black);
}

TEST(Game_LastMoveFollowsEvent) {
    // 1 行目に打ち、1 行目スライドで印が追従する
    Board b = parseBoard({
        ".WB.......",
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
    Game g(b, Cell::Black, 3, BoardEvent{EventKind::SlideRow, 0, EventDir::Negative}, 4);
    g.play({0, 0});
    CHECK(g.lastMove() == Pos{0, 0});
    g.fireDueEvent();
    CHECK(g.lastMove() == Pos{9, 0});  // A1 → J1（回り込み）
}

TEST(Game_WinnerAndDraw) {
    Board b;
    b.set({0, 0}, Cell::Black);
    b.set({1, 0}, Cell::White);
    const Game draw(b, Cell::Black, 0, kRow0Right, 1);
    CHECK(!draw.winner().has_value());
    b.set({2, 0}, Cell::White);
    const Game white(b, Cell::Black, 0, kRow0Right, 1);
    CHECK(white.winner() == Cell::White);
}

TEST(Game_RandomPlayoutsKeepInvariants) {
    // ランダム対局を多数回し、状態遷移と不変条件を確認
    for (std::uint32_t seed = 1; seed <= 30; ++seed) {
        Game g(seed);
        Random rng(seed * 31u);
        int steps = 0;
        while (g.status() != TurnStatus::GameOver && steps < 1000) {
            ++steps;
            switch (g.status()) {
            case TurnStatus::NeedMove: {
                const auto moves = g.legalMoves();
                g.play(moves[rng.below(static_cast<std::uint32_t>(moves.size()))]);
                break;
            }
            case TurnStatus::MustPass:
                g.pass();
                break;
            case TurnStatus::EventDue: {
                const int b = g.count(Cell::Black);
                const int w = g.count(Cell::White);
                g.fireDueEvent();
                CHECK(g.count(Cell::Black) == b && g.count(Cell::White) == w);
                break;
            }
            case TurnStatus::BothBlocked:
                g.resolveBothBlocked();
                break;
            case TurnStatus::GameOver:
                break;
            }
            CHECK(g.counter() >= 0 && g.counter() <= 4);
        }
        CHECK(g.status() == TurnStatus::GameOver);
    }
}
