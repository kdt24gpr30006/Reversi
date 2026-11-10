#include "TestFramework.h"

#include <algorithm>
#include <chrono>
#include <cstdio>

#include "shift/Ai.h"
#include "shift/EventPicker.h"

using namespace shift;
using Clock = std::chrono::steady_clock;

namespace {

long long elapsedMs(Clock::time_point start) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start).count();
}

// ランダム対局を進め、空きが targetEmpties かつ手番側の合法手が 2 つ以上の局面を返す
std::optional<Game> randomPosition(std::uint32_t seed, int targetEmpties) {
    Game g(seed);
    Random rng(seed + 1000u);
    while (g.status() != TurnStatus::GameOver) {
        switch (g.status()) {
        case TurnStatus::NeedMove: {
            const auto moves = g.legalMoves();
            if (g.board().emptyCount() == targetEmpties) {
                if (moves.size() >= 2) return g;
                return std::nullopt;
            }
            g.play(moves[rng.below(static_cast<std::uint32_t>(moves.size()))]);
            break;
        }
        case TurnStatus::MustPass: g.pass(); break;
        case TurnStatus::EventDue: g.fireDueEvent(); break;
        case TurnStatus::BothBlocked: g.resolveBothBlocked(); break;
        case TurnStatus::GameOver: break;
        }
        if (g.board().emptyCount() < targetEmpties) return std::nullopt;
    }
    return std::nullopt;
}

// 独立実装の完全読み（予告イベント 1 件のみ、以降のイベントなし）。手番側から見た最終石差。
int solve(Board b, Color turn, int counter, bool pending, const BoardEvent& ev) {
    const auto advance = [&](Board& nb, Color& nt, int& nc, bool& np) {
        nt = opponent(nt);
        if (nb.isFull()) return;
        if (++nc >= 4) {
            if (np) {
                ev.apply(nb);
                np = false;
            }
            nc = 0;
        }
    };
    if (b.isFull()) return b.count(turn) - b.count(opponent(turn));
    const auto moves = b.legalMoves(turn);
    if (moves.empty()) {
        if (!b.hasLegalMove(opponent(turn))) {
            if (!pending) return b.count(turn) - b.count(opponent(turn));
            ev.apply(b);
            return solve(b, turn, 0, false, ev);
        }
        Color nt = turn;
        int nc = counter;
        bool np = pending;
        advance(b, nt, nc, np);
        return -solve(b, nt, nc, np, ev);
    }
    int best = -1000;
    for (const Pos m : moves) {
        Board nb = b;
        nb.place(m, turn);
        Color nt = turn;
        int nc = counter;
        bool np = pending;
        advance(nb, nt, nc, np);
        best = std::max(best, -solve(nb, nt, nc, np, ev));
    }
    return best;
}

int valueOfMove(const Game& g, Pos m, bool withEvent) {
    Board nb = g.board();
    nb.place(m, g.turn());
    Color nt = opponent(g.turn());
    int nc = g.counter() + 1;
    bool np = withEvent;
    if (!nb.isFull() && nc >= 4) {
        if (np) {
            g.nextEvent().apply(nb);
            np = false;
        }
        nc = 0;
    }
    return -solve(nb, nt, nc, np, g.nextEvent());
}

}  // namespace

TEST(Ai_ReturnsLegalMoveForAllLevels) {
    const Game g(55);
    for (Difficulty d : {Difficulty::Weak, Difficulty::Normal, Difficulty::Strong}) {
        AiOptions opt;
        opt.difficulty = d;
        opt.timeLimit = std::chrono::milliseconds(300);
        Random rng(1);
        const auto m = chooseMove(g, opt, rng);
        CHECK(m.has_value() && g.isLegal(*m));
    }
}

TEST(Ai_NoLegalMoveReturnsNullopt) {
    Board b;
    b.set({0, 0}, Cell::White);
    b.set({1, 0}, Cell::Black);
    const Game g(b, Cell::Black, 0, BoardEvent{}, 1);
    for (Difficulty d : {Difficulty::Weak, Difficulty::Normal, Difficulty::Strong}) {
        AiOptions opt;
        opt.difficulty = d;
        Random rng(1);
        CHECK(!chooseMove(g, opt, rng).has_value());
    }
}

TEST(Ai_CancelReturnsQuickly) {
    const Game g(77);
    std::atomic<bool> cancel{true};
    AiOptions opt;
    opt.difficulty = Difficulty::Strong;
    opt.cancel = &cancel;
    Random rng(1);
    const auto start = Clock::now();
    const auto m = chooseMove(g, opt, rng);
    CHECK(elapsedMs(start) < 300);
    CHECK(m.has_value() && g.isLegal(*m));
}

TEST(Ai_StrongWithinThreeSeconds) {
    // 本番設定（既定 2800ms）で 3 秒以内に着手する（SC-003）
    for (int targetEmpties : {96, 70, 40}) {
        std::optional<Game> g;
        for (std::uint32_t seed = 1; !g && seed < 200; ++seed) g = randomPosition(seed, targetEmpties);
        CHECK(g.has_value());
        if (!g) continue;
        AiOptions opt;
        opt.difficulty = Difficulty::Strong;
        Random rng(1);
        const auto start = Clock::now();
        const auto m = chooseMove(*g, opt, rng);
        const long long ms = elapsedMs(start);
        std::printf("    strong: empties=%d time=%lldms depth=%d nodes=%lld\n", targetEmpties, ms,
                    lastAiStats().completedDepth, lastAiStats().nodes);
        CHECK(ms <= 3000);
        CHECK(m.has_value() && g->isLegal(*m));
    }
}

TEST(Ai_NormalIsFast) {
    std::optional<Game> g;
    for (std::uint32_t seed = 1; !g && seed < 200; ++seed) g = randomPosition(seed, 60);
    CHECK(g.has_value());
    if (!g) return;
    AiOptions opt;
    opt.difficulty = Difficulty::Normal;
    Random rng(1);
    const auto start = Clock::now();
    const auto m = chooseMove(*g, opt, rng);
    CHECK(elapsedMs(start) <= 3000);
    CHECK(m.has_value() && g->isLegal(*m));
}

TEST(Ai_StrongReadsScheduledEventInEndgame) {
    // 空き 4 マス・2 手後にイベント発動の終盤で、強 CPU の手が完全読みの最善値に一致することを確認
    int checked = 0;
    int eventMatters = 0;
    for (std::uint32_t seed = 1; seed < 3000 && checked < 40; ++seed) {
        const auto pos = randomPosition(seed, 4);
        if (!pos) continue;
        Random erng(seed);
        const BoardEvent ev = pickEvent(pos->board(), erng);
        const Game g(pos->board(), pos->turn(), 2, ev, seed);
        if (g.status() != TurnStatus::NeedMove) continue;

        const auto moves = g.legalMoves();
        int bestWith = -1000;
        int bestWithout = -1000;
        Pos argWithout = moves.front();
        for (const Pos m : moves) {
            bestWith = std::max(bestWith, valueOfMove(g, m, true));
            const int v = valueOfMove(g, m, false);
            if (v > bestWithout) {
                bestWithout = v;
                argWithout = m;
            }
        }
        if (valueOfMove(g, argWithout, true) < bestWith) ++eventMatters;

        AiOptions opt;
        opt.difficulty = Difficulty::Strong;
        opt.timeLimit = std::chrono::milliseconds(1000);
        Random rng(1);
        const auto m = chooseMove(g, opt, rng);
        CHECK(m.has_value());
        if (m) CHECK(valueOfMove(g, *m, true) == bestWith);
        ++checked;
    }
    std::printf("    endgame positions=%d (event changes best move in %d)\n", checked, eventMatters);
    CHECK(checked >= 20);
    CHECK(eventMatters >= 1);
}
