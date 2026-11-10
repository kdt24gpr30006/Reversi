#pragma once
#include <cstdint>
#include <random>

namespace shift {

// シード注入可能な乱数。区間変換を自前実装し、標準ライブラリ実装に依存せず結果を再現できる（FR-014）。
class Random {
public:
    explicit Random(std::uint32_t seed) : engine_(seed) {}

    std::uint32_t next() { return static_cast<std::uint32_t>(engine_()); }

    // 0..n-1 を一様に返す（n==0 のときは 0）
    std::uint32_t below(std::uint32_t n) {
        if (n <= 1) return 0;
        const std::uint32_t threshold = (0u - n) % n;  // 2^32 mod n
        for (;;) {
            const std::uint32_t r = next();
            if (r >= threshold) return r % n;
        }
    }

    // percent% の確率で true
    bool chance(int percent) { return static_cast<int>(below(100)) < percent; }

private:
    std::mt19937 engine_;
};

}  // namespace shift
