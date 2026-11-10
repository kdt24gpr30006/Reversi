#include "TestFramework.h"

#include "shift/EventPicker.h"

using namespace shift;

TEST(Picker_NeverPicksNoChangeEvent) {
    const Board b = Board::initial();
    for (std::uint32_t seed = 0; seed < 300; ++seed) {
        Random rng(seed);
        const BoardEvent e = pickEvent(b, rng);
        CHECK(!(e.applied(b) == b));
    }
}

TEST(Picker_FallsBackToOtherKindWhenOneKindAllExcluded) {
    // 左上区画の中心だけに石 → 全回転が無変化、スライドは変化あり
    Board b;
    b.set({2, 2}, Cell::Black);
    for (std::uint32_t seed = 0; seed < 200; ++seed) {
        Random rng(seed);
        const BoardEvent e = pickEvent(b, rng);
        CHECK(e.kind != EventKind::Rotate);
        CHECK(!(e.applied(b) == b));
    }
}

TEST(Picker_FallsBackToAllWhenEverythingExcluded) {
    const Board empty;
    bool sawSlide = false;
    bool sawRotate = false;
    const auto all = BoardEvent::allCandidates();
    for (std::uint32_t seed = 0; seed < 300; ++seed) {
        Random rng(seed);
        const BoardEvent e = pickEvent(empty, rng);
        CHECK(std::find(all.begin(), all.end(), e) != all.end());
        (e.kind == EventKind::Rotate ? sawRotate : sawSlide) = true;
    }
    CHECK(sawSlide);
    CHECK(sawRotate);
}

TEST(Picker_SameSeedSameSequence) {
    const Board b = Board::initial();
    Random r1(42);
    Random r2(42);
    for (int i = 0; i < 50; ++i) CHECK(pickEvent(b, r1) == pickEvent(b, r2));
}

TEST(Picker_KindRatioIsAboutHalf) {
    const Board b = Board::initial();  // 初期盤ではスライド 8 件・回転 8 件が有効
    Random rng(2026);
    int rotations = 0;
    constexpr int kTrials = 4000;
    for (int i = 0; i < kTrials; ++i) {
        if (pickEvent(b, rng).kind == EventKind::Rotate) ++rotations;
    }
    CHECK(rotations > kTrials * 45 / 100);
    CHECK(rotations < kTrials * 55 / 100);
}
