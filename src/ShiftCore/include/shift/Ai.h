#pragma once
#include <atomic>
#include <chrono>
#include <optional>

#include "shift/Game.h"

namespace shift {

enum class Difficulty { Weak, Normal, Strong };

struct AiOptions {
    Difficulty difficulty = Difficulty::Normal;
    std::chrono::milliseconds timeLimit{2800};  // Strong の締切（3 秒上限に余裕を持たせる）
    int normalDepth = 3;                        // Normal の読みの深さ
    const std::atomic<bool>* cancel = nullptr;  // true で即打ち切り
};

// 手番側の手を選ぶ（FR-019）。合法手が無ければ nullopt。
std::optional<Pos> chooseMove(const Game& game, const AiOptions& options, Random& rng);

// 探索の統計（テスト・ベンチ用）
struct AiStats {
    int completedDepth = 0;
    long long nodes = 0;
};
const AiStats& lastAiStats();

}  // namespace shift
