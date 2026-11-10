#include "shift/EventPicker.h"

namespace shift {

BoardEvent pickEvent(const Board& board, Random& rng) {
    const std::vector<BoardEvent> all = BoardEvent::allCandidates();
    std::vector<BoardEvent> slides;
    std::vector<BoardEvent> rotations;
    for (const BoardEvent& e : all) {
        if (e.applied(board) == board) continue;  // 無変化は除外
        (e.kind == EventKind::Rotate ? rotations : slides).push_back(e);
    }

    const auto pickFrom = [&rng](const std::vector<BoardEvent>& v) {
        return v[rng.below(static_cast<std::uint32_t>(v.size()))];
    };

    if (!slides.empty() && !rotations.empty()) {
        return rng.chance(50) ? pickFrom(slides) : pickFrom(rotations);
    }
    if (!slides.empty()) return pickFrom(slides);
    if (!rotations.empty()) return pickFrom(rotations);
    return pickFrom(all);
}

}  // namespace shift
