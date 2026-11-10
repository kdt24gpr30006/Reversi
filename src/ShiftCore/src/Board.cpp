#include "shift/Board.h"

#include <algorithm>

namespace shift {

namespace {
constexpr int kDirs[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

// 1 方向で挟める石数（挟めなければ 0）
int lineFlips(const Board& b, Pos p, Color color, int dx, int dy) {
    const Color opp = opponent(color);
    int x = p.x + dx;
    int y = p.y + dy;
    int n = 0;
    while (inBounds(x, y) && b.at(x, y) == opp) {
        x += dx;
        y += dy;
        ++n;
    }
    if (n > 0 && inBounds(x, y) && b.at(x, y) == color) return n;
    return 0;
}
}  // namespace

Board Board::initial() {
    Board b;
    b.set({4, 4}, Cell::White);
    b.set({5, 4}, Cell::Black);
    b.set({4, 5}, Cell::Black);
    b.set({5, 5}, Cell::White);
    return b;
}

std::vector<Pos> Board::flipsFor(Pos p, Color color) const {
    std::vector<Pos> out;
    if (!inBounds(p.x, p.y) || at(p) != Cell::Empty) return out;
    for (const auto& d : kDirs) {
        const int n = lineFlips(*this, p, color, d[0], d[1]);
        for (int i = 1; i <= n; ++i) out.push_back({p.x + d[0] * i, p.y + d[1] * i});
    }
    return out;
}

bool Board::isLegal(Pos p, Color color) const {
    if (!inBounds(p.x, p.y) || at(p) != Cell::Empty) return false;
    for (const auto& d : kDirs) {
        if (lineFlips(*this, p, color, d[0], d[1]) > 0) return true;
    }
    return false;
}

std::vector<Pos> Board::legalMoves(Color color) const {
    std::vector<Pos> out;
    for (int i = 0; i < kCells; ++i) {
        const Pos p = fromIndex(i);
        if (isLegal(p, color)) out.push_back(p);
    }
    return out;
}

bool Board::hasLegalMove(Color color) const {
    for (int i = 0; i < kCells; ++i) {
        if (isLegal(fromIndex(i), color)) return true;
    }
    return false;
}

std::vector<Pos> Board::place(Pos p, Color color) {
    std::vector<Pos> flips = flipsFor(p, color);
    if (flips.empty()) return flips;
    set(p, color);
    for (const Pos& f : flips) set(f, color);
    return flips;
}

int Board::count(Cell c) const {
    return static_cast<int>(std::count(cells_.begin(), cells_.end(), c));
}

}  // namespace shift
