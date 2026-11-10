#pragma once
#include <array>
#include <vector>

#include "shift/Types.h"

namespace shift {

// 10×10 の盤面。挟み判定は盤端で打ち切り、回り込みはしない（FR-003）。
class Board {
public:
    // E5=白, F5=黒, E6=黒, F6=白（FR-001）
    static Board initial();

    Cell at(Pos p) const { return cells_[static_cast<size_t>(toIndex(p))]; }
    Cell at(int x, int y) const { return cells_[static_cast<size_t>(y * kSize + x)]; }
    void set(Pos p, Cell c) { cells_[static_cast<size_t>(toIndex(p))] = c; }

    // color が p に置いた場合に反転する石。空なら非合法手。
    std::vector<Pos> flipsFor(Pos p, Color color) const;
    bool isLegal(Pos p, Color color) const;
    std::vector<Pos> legalMoves(Color color) const;
    bool hasLegalMove(Color color) const;

    // 着手して反転した石を返す。非合法なら何もせず空を返す。
    std::vector<Pos> place(Pos p, Color color);

    int count(Cell c) const;
    int emptyCount() const { return count(Cell::Empty); }
    bool isFull() const { return emptyCount() == 0; }

    bool operator==(const Board&) const = default;

private:
    std::array<Cell, kCells> cells_{};
};

}  // namespace shift
