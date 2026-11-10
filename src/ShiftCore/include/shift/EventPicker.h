#pragma once
#include "shift/BoardEvent.h"
#include "shift/Random.h"

namespace shift {

// 次の予告イベントを抽選する（FR-012, FR-013）。
// 種別（スライド/回転）を 50% ずつ選び、種別内で一様。盤が変化しない候補は除外。
BoardEvent pickEvent(const Board& board, Random& rng);

}  // namespace shift
