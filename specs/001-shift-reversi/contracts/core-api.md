# Contract: ShiftCore 公開 API

プレゼンテーション層（ShiftReversi）とテスト（ShiftTests）が依存してよいのはこのヘッダ群のみ。
名前空間 `shift`。DxLib のヘッダ・型を含んではならない（憲章 I）。

## Headers

| ヘッダ | 主な宣言 |
|--------|---------|
| `shift/Types.h` | `enum class Cell`, `using Color = Cell`, `opponent()`, `struct Pos`, `kSize = 10` |
| `shift/Board.h` | `class Board` |
| `shift/BoardEvent.h` | `struct BoardEvent`, `apply`, `mapPos`, `allCandidates`, `describe` |
| `shift/Random.h` | `class Random` |
| `shift/EventPicker.h` | `BoardEvent pickEvent(const Board&, Random&)` |
| `shift/Game.h` | `class Game`, `enum class TurnStatus`, `struct MoveResult`, `struct EventResult` |
| `shift/Ai.h` | `enum class Difficulty`, `struct AiOptions`, `std::optional<Pos> chooseMove(...)` |

## Game

```cpp
enum class TurnStatus { NeedMove, MustPass, EventDue, BothBlocked, GameOver };

struct MoveResult  { Pos placed; std::vector<Pos> flipped; bool eventDue; bool gameOver; };
struct EventResult { Board before; BoardEvent event; bool forced; bool gameOver; };

class Game {
public:
    Game(uint32_t seed);                       // 初期配置・黒番・counter 0・最初の予告抽選
    TurnStatus status() const;
    const Board& board() const;
    Color turn() const;
    int counter() const;                       // 0..4
    int movesUntilEvent() const;               // 4 - counter
    const BoardEvent& nextEvent() const;
    std::optional<Pos> lastMove() const;
    bool isLegal(Pos) const;
    std::vector<Pos> legalMoves() const;       // 手番側
    std::vector<Pos> flipsFor(Pos) const;      // 手番側のホバー表示用（FR-024b）
    MoveResult  play(Pos);                     // 前提: status()==NeedMove && isLegal
    void        pass();                        // 前提: status()==MustPass
    EventResult fireDueEvent();                // 前提: status()==EventDue
    EventResult resolveBothBlocked();          // 前提: status()==BothBlocked
};
```

前提違反は `assert` で検出（Release では無視せず何もしない）。

## AI

```cpp
enum class Difficulty { Weak, Normal, Strong };
struct AiOptions {
    Difficulty difficulty;
    std::chrono::milliseconds timeLimit{2800};  // Strong の締切
    const std::atomic<bool>* cancel = nullptr;  // true で即打ち切り
};
// 手番側に合法手が無ければ nullopt。Game はコピーで渡す（スレッド安全）。
std::optional<Pos> chooseMove(const Game& game, const AiOptions& opt, Random& rng);
```

## ShiftTests CLI

| コマンド | 動作 | 終了コード |
|----------|------|-----------|
| `ShiftTests.exe` | 全ルール/AI テストを実行 | 0=全成功, 1=失敗あり |
| `ShiftTests.exe --bench N [--strong-ms M]` | 強/中 vs 弱 の自己対戦を各 N 局（先後交互）、勝率を表示 | 0 |
