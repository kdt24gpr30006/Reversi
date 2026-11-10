#include "shift/Game.h"

#include <cassert>

#include "shift/EventPicker.h"

namespace shift {

Game::Game(std::uint32_t seed) : board_(Board::initial()), rng_(seed) {
    next_ = pickEvent(board_, rng_);  // FR-007
}

Game::Game(const Board& board, Color turn, int counter, const BoardEvent& next, std::uint32_t seed)
    : board_(board), turn_(turn), counter_(counter), next_(next), rng_(seed) {
    eventDue_ = counter_ >= kEventInterval;
}

TurnStatus Game::status() const {
    if (over_) return TurnStatus::GameOver;
    if (eventDue_) return TurnStatus::EventDue;
    if (board_.hasLegalMove(turn_)) return TurnStatus::NeedMove;
    if (board_.hasLegalMove(opponent(turn_))) return TurnStatus::MustPass;
    return TurnStatus::BothBlocked;
}

std::optional<Color> Game::winner() const {
    const int b = board_.count(Cell::Black);
    const int w = board_.count(Cell::White);
    if (b == w) return std::nullopt;
    return b > w ? Cell::Black : Cell::White;
}

MoveResult Game::play(Pos p) {
    MoveResult r;
    r.placed = p;
    if (status() != TurnStatus::NeedMove || !isLegal(p)) {
        assert(false && "Game::play precondition");
        return r;
    }
    r.flipped = board_.place(p, turn_);
    last_ = p;
    turn_ = opponent(turn_);
    ++counter_;
    if (board_.isFull()) {
        over_ = true;  // 満杯はイベントなしで終局（FR-017）
    } else if (counter_ >= kEventInterval) {
        eventDue_ = true;
    }
    r.eventDue = eventDue_;
    r.gameOver = over_;
    return r;
}

void Game::pass() {
    if (status() != TurnStatus::MustPass) {
        assert(false && "Game::pass precondition");
        return;
    }
    turn_ = opponent(turn_);
    ++counter_;  // パスも 1 手（FR-005）
    if (counter_ >= kEventInterval) eventDue_ = true;
}

EventResult Game::applyNextEvent(bool forced) {
    EventResult r;
    r.before = board_;
    r.event = next_;
    r.forced = forced;
    next_.apply(board_);
    if (last_) last_ = next_.mapPos(*last_);  // FR-024a
    counter_ = 0;
    eventDue_ = false;
    next_ = pickEvent(board_, rng_);
    return r;
}

EventResult Game::fireDueEvent() {
    if (status() != TurnStatus::EventDue) {
        assert(false && "Game::fireDueEvent precondition");
        return {};
    }
    return applyNextEvent(false);
}

EventResult Game::resolveBothBlocked() {
    if (status() != TurnStatus::BothBlocked) {
        assert(false && "Game::resolveBothBlocked precondition");
        return {};
    }
    EventResult r = applyNextEvent(true);  // パスは記録しない（FR-008）
    if (!board_.hasLegalMove(turn_) && !board_.hasLegalMove(opponent(turn_))) {
        over_ = true;
        r.gameOver = true;
    }
    return r;
}

}  // namespace shift
