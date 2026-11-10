#pragma once
#include <cstdint>
#include <string>

namespace shift {

constexpr int kSize = 10;
constexpr int kCells = kSize * kSize;

enum class Cell : std::uint8_t { Empty, Black, White };
using Color = Cell;  // Black / White のみを使う

constexpr Color opponent(Color c) { return c == Cell::Black ? Cell::White : Cell::Black; }

// x = 列 0..9（A..J）, y = 行 0..9（1..10）
struct Pos {
    int x = 0;
    int y = 0;
    friend constexpr bool operator==(const Pos&, const Pos&) = default;
};

constexpr bool inBounds(int x, int y) { return x >= 0 && x < kSize && y >= 0 && y < kSize; }
constexpr int toIndex(Pos p) { return p.y * kSize + p.x; }
constexpr Pos fromIndex(int i) { return {i % kSize, i / kSize}; }

// 例: {4,4} -> "E5"
inline std::string posName(Pos p) {
    std::string s(1, static_cast<char>('A' + p.x));
    s += std::to_string(p.y + 1);
    return s;
}

}  // namespace shift
