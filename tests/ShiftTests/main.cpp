// ShiftTests: ルール自動テスト（既定）と CPU 自己対戦ベンチ（--bench）
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <windows.h>

#include "TestFramework.h"
#include "shift/Ai.h"

using namespace shift;

namespace {

int runTests() {
    int failedTests = 0;
    for (const auto& t : testfw::registry()) {
        const int before = testfw::failures();
        std::printf("[ RUN  ] %s\n", t.name);
        t.fn();
        const bool ok = testfw::failures() == before;
        std::printf("[ %s ] %s\n", ok ? "PASS" : "FAIL", t.name);
        if (!ok) ++failedTests;
    }
    const size_t total = testfw::registry().size();
    std::printf("\n%zu tests, %zu passed, %d failed\n", total, total - static_cast<size_t>(failedTests), failedTests);
    return failedTests == 0 ? 0 : 1;
}

const char* name(Difficulty d) {
    switch (d) {
    case Difficulty::Weak: return "弱";
    case Difficulty::Normal: return "中";
    case Difficulty::Strong: return "強";
    }
    return "?";
}

// 1 局対戦し、黒から見た結果（+1 勝 / 0 引分 / -1 負）を返す
int playMatch(Difficulty black, Difficulty white, std::uint32_t seed, int strongMs) {
    Game g(seed);
    Random rng(seed ^ 0x9E3779B9u);
    while (g.status() != TurnStatus::GameOver) {
        switch (g.status()) {
        case TurnStatus::NeedMove: {
            AiOptions opt;
            opt.difficulty = g.turn() == Cell::Black ? black : white;
            opt.timeLimit = std::chrono::milliseconds(strongMs);
            const auto m = chooseMove(g, opt, rng);
            g.play(*m);
            break;
        }
        case TurnStatus::MustPass: g.pass(); break;
        case TurnStatus::EventDue: g.fireDueEvent(); break;
        case TurnStatus::BothBlocked: g.resolveBothBlocked(); break;
        case TurnStatus::GameOver: break;
        }
    }
    const auto w = g.winner();
    if (!w) return 0;
    return *w == Cell::Black ? 1 : -1;
}

void benchPair(Difficulty a, Difficulty b, int games, int strongMs) {
    int wins = 0, draws = 0, losses = 0;
    for (int i = 0; i < games; ++i) {
        const bool aIsBlack = i % 2 == 0;
        const int r = aIsBlack ? playMatch(a, b, 1000u + static_cast<std::uint32_t>(i), strongMs)
                               : -playMatch(b, a, 1000u + static_cast<std::uint32_t>(i), strongMs);
        if (r > 0) ++wins;
        else if (r == 0) ++draws;
        else ++losses;
    }
    std::printf("%s vs %s: %d勝 %d分 %d敗 / %d局  勝率 %.1f%%\n", name(a), name(b), wins, draws, losses, games,
                100.0 * wins / games);
}

}  // namespace

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);  // 日本語出力
    int benchGames = 0;
    int strongMs = 200;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--bench") == 0 && i + 1 < argc) benchGames = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--strong-ms") == 0 && i + 1 < argc) strongMs = std::atoi(argv[++i]);
    }
    if (benchGames > 0) {
        std::printf("ベンチ: 各 %d 局（先後交互）, 強の思考 %dms\n", benchGames, strongMs);
        benchPair(Difficulty::Strong, Difficulty::Weak, benchGames, strongMs);
        benchPair(Difficulty::Normal, Difficulty::Weak, benchGames, strongMs);
        return 0;
    }
    return runTests();
}
