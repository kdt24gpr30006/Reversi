#pragma once
#include <optional>
#include <vector>

#include "shift/Board.h"
#include "shift/BoardEvent.h"
#include "shift/Random.h"

namespace shift {

enum class TurnStatus { NeedMove, MustPass, EventDue, BothBlocked, GameOver };

struct MoveResult {
    Pos placed;
    std::vector<Pos> flipped;
    bool eventDue = false;
    bool gameOver = false;
};

struct EventResult {
    Board before;       // 適用前の盤（アニメーション用）
    BoardEvent event;   // 適用したイベント
    bool forced = false;     // 両者合法手なしによる前倒し発動か
    bool gameOver = false;
};

// 対局の進行。プレゼンテーション層はステップ単位で呼び出し、結果をアニメーションする。
class Game {
public:
    static constexpr int kEventInterval = 4;

    explicit Game(std::uint32_t seed);
    // テスト・AI 用: 任意の状態から開始
    Game(const Board& board, Color turn, int counter, const BoardEvent& next, std::uint32_t seed);

    TurnStatus status() const;
    const Board& board() const { return board_; }
    Color turn() const { return turn_; }
    int counter() const { return counter_; }
    int movesUntilEvent() const { return kEventInterval - counter_; }
    const BoardEvent& nextEvent() const { return next_; }
    std::optional<Pos> lastMove() const { return last_; }
    bool isOver() const { return over_; }

    bool isLegal(Pos p) const { return board_.isLegal(p, turn_); }
    std::vector<Pos> legalMoves() const { return board_.legalMoves(turn_); }
    std::vector<Pos> flipsFor(Pos p) const { return board_.flipsFor(p, turn_); }

    int count(Color c) const { return board_.count(c); }
    // 勝者。引き分けは nullopt（FR-018）
    std::optional<Color> winner() const;

    MoveResult play(Pos p);            // 前提: status()==NeedMove && isLegal(p)
    void pass();                       // 前提: status()==MustPass
    EventResult fireDueEvent();        // 前提: status()==EventDue
    EventResult resolveBothBlocked();  // 前提: status()==BothBlocked

private:
    EventResult applyNextEvent(bool forced);

    Board board_;
    Color turn_ = Cell::Black;
    int counter_ = 0;
    BoardEvent next_;
    std::optional<Pos> last_;
    bool eventDue_ = false;
    bool over_ = false;
    Random rng_;
};

}  // namespace shift
