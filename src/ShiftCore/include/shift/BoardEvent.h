#pragma once
#include <string>
#include <vector>

#include "shift/Board.h"

namespace shift {

enum class EventKind : std::uint8_t { SlideRow, SlideCol, Rotate };

// SlideRow: Positive=右, Negative=左
// SlideCol: Positive=下, Negative=上
// Rotate  : Positive=時計回り, Negative=反時計回り
enum class EventDir : std::uint8_t { Positive, Negative };

constexpr int kQuadrantSize = 5;
constexpr int kQuadrantCount = 4;  // 0=左上 1=右上 2=左下 3=右下

// 盤面イベント（FR-009〜011）
struct BoardEvent {
    EventKind kind = EventKind::SlideRow;
    int index = 0;  // 行 / 列 / 区画番号
    EventDir dir = EventDir::Positive;

    friend bool operator==(const BoardEvent&, const BoardEvent&) = default;

    // 適用前に p にあった内容が適用後に移る位置（対象外は恒等）
    Pos mapPos(Pos p) const;
    // p がイベントの対象範囲に含まれるか
    bool affects(Pos p) const;

    void apply(Board& board) const;
    Board applied(const Board& board) const;

    // 予告表示用の日本語説明（FR-016）
    std::string describe() const;

    // 全 48 候補（スライド 40 + 回転 8）
    static std::vector<BoardEvent> allCandidates();
};

// 区画の左上座標
constexpr Pos quadrantOrigin(int q) { return {(q % 2) * kQuadrantSize, (q / 2) * kQuadrantSize}; }

}  // namespace shift
