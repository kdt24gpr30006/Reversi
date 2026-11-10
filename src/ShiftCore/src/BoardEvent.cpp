#include "shift/BoardEvent.h"

namespace shift {

bool BoardEvent::affects(Pos p) const {
    switch (kind) {
    case EventKind::SlideRow:
        return p.y == index;
    case EventKind::SlideCol:
        return p.x == index;
    case EventKind::Rotate: {
        const Pos o = quadrantOrigin(index);
        return p.x >= o.x && p.x < o.x + kQuadrantSize && p.y >= o.y && p.y < o.y + kQuadrantSize;
    }
    }
    return false;
}

Pos BoardEvent::mapPos(Pos p) const {
    if (!affects(p)) return p;
    const int step = dir == EventDir::Positive ? 1 : -1;
    switch (kind) {
    case EventKind::SlideRow:
        return {(p.x + step + kSize) % kSize, p.y};
    case EventKind::SlideCol:
        return {p.x, (p.y + step + kSize) % kSize};
    case EventKind::Rotate: {
        const Pos o = quadrantOrigin(index);
        const int u = p.x - o.x;
        const int v = p.y - o.y;
        constexpr int m = kQuadrantSize - 1;
        // 画面座標（y 下向き）での時計回り: (u,v) -> (m-v, u)
        if (dir == EventDir::Positive) return {o.x + (m - v), o.y + u};
        return {o.x + v, o.y + (m - u)};
    }
    }
    return p;
}

void BoardEvent::apply(Board& board) const { board = applied(board); }

Board BoardEvent::applied(const Board& board) const {
    Board out;
    for (int i = 0; i < kCells; ++i) {
        const Pos p = fromIndex(i);
        out.set(mapPos(p), board.at(p));
    }
    return out;
}

std::string BoardEvent::describe() const {
    const bool pos = dir == EventDir::Positive;
    switch (kind) {
    case EventKind::SlideRow:
        return std::to_string(index + 1) + "行目を" + (pos ? "→ 右へ" : "← 左へ") + "1マススライド";
    case EventKind::SlideCol:
        return std::string(1, static_cast<char>('A' + index)) + "列を" + (pos ? "↓ 下へ" : "↑ 上へ") +
               "1マススライド";
    case EventKind::Rotate: {
        static const char* kNames[kQuadrantCount] = {"左上", "右上", "左下", "右下"};
        return std::string(kNames[index]) + "区画を" + (pos ? "時計回り" : "反時計回り") + "に90°回転";
    }
    }
    return {};
}

std::vector<BoardEvent> BoardEvent::allCandidates() {
    std::vector<BoardEvent> out;
    for (EventDir d : {EventDir::Positive, EventDir::Negative}) {
        for (int i = 0; i < kSize; ++i) out.push_back({EventKind::SlideRow, i, d});
        for (int i = 0; i < kSize; ++i) out.push_back({EventKind::SlideCol, i, d});
    }
    for (EventDir d : {EventDir::Positive, EventDir::Negative}) {
        for (int q = 0; q < kQuadrantCount; ++q) out.push_back({EventKind::Rotate, q, d});
    }
    return out;
}

}  // namespace shift
