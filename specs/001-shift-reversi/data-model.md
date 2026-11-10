# Data Model: SHIFT REVERSI

すべて `ShiftCore`（DxLib 非依存）に属する。座標は `x`=列 0..9（A..J）、`y`=行 0..9（1..10）、
インデックス `y*10+x`。

## Cell / Color

| 値 | 意味 |
|----|------|
| Empty | 空き |
| Black | 黒（先手） |
| White | 白 |

`opponent(Black)=White`, `opponent(White)=Black`。

## Pos

| フィールド | 型 | 制約 |
|-----------|----|------|
| x | int | 0..9 |
| y | int | 0..9 |

表示名: `"ABCDEFGHIJ"[x]` + `(y+1)`（例: E5 = {4,4}）。

## Board

| フィールド | 型 | 説明 |
|-----------|----|------|
| cells | array<Cell,100> | 各マス |

- 初期配置（`Board::initial()`）: E5={4,4}=White, F5={5,4}=Black, E6={4,5}=Black, F6={5,5}=White。
- 不変条件: 盤面イベント適用前後で `count(Black)`, `count(White)` が不変（FR-011）。
- 操作: `flipsFor(pos,color)`（反転対象リスト、空なら非合法）、`legalMoves(color)`、`place(pos,color)`、
  `count(color)`、`isFull()`。挟み判定は盤端で打ち切り（FR-003）。

## BoardEvent

| フィールド | 型 | 値 |
|-----------|----|----|
| kind | enum | SlideRow / SlideCol / Rotate |
| index | int | SlideRow: 行 0..9、SlideCol: 列 0..9、Rotate: 区画 0=左上 1=右上 2=左下 3=右下 |
| dir | enum | SlideRow: Right/Left、SlideCol: Down/Up、Rotate: CW/CCW |

- `apply(Board&)`: FR-009（回り込みあり 1 マス）、FR-010（区画 5×5 を 90°）。
- `mapPos(Pos) -> Pos`: 適用前のマスが適用後にどこへ移るか（対象外は恒等）。直前着手の印の追従（FR-024a）と
  アニメーションに使う。
- `allCandidates()`: 40 スライド + 8 回転 = 48 件。
- `describe()`: 日本語説明（例「3 行目 → 右へスライド」「左上区画 ↻ 時計回り」）（FR-016）。

回転写像（区画原点 (ox,oy)、局所座標 (u,v)、0..4）:
- CW: (u,v) → (4−v, u)
- CCW: (u,v) → (v, 4−u)

## EventPicker

`pick(const Board&, Random&) -> BoardEvent`（FR-012, FR-013）
1. 48 候補のうち `apply` で盤が変化するものを種別ごとに集める。
2. 両種別に候補あり → 50% で種別を選択、種別内一様。片方のみ → その種別内一様。両方空 → 48 件から一様。

## Random

`explicit Random(uint32_t seed)`、`uint32_t below(uint32_t n)`（0..n−1、リジェクション法）、`bool chance(int percent)`。

## Game（対局状態）

| フィールド | 型 | 説明 |
|-----------|----|------|
| board | Board | 現在の盤 |
| turn | Color | 手番 |
| counter | int | 手数カウンタ 0..4 |
| nextEvent | BoardEvent | 予告 |
| lastMove | optional<Pos> | 直前着手（イベントで追従） |
| eventDue | bool | counter==4 でイベント発動待ち |
| over | bool | 終局 |
| rng | Random | 抽選用 |

### 状態遷移

```text
status():
  over                        → GameOver
  eventDue                    → EventDue
  legalMoves(turn) 非空       → NeedMove
  legalMoves(opponent) 非空   → MustPass
  それ以外                    → BothBlocked

play(pos)  [NeedMove]  : 着手・反転, counter++, lastMove=pos, turn 交代
                         board 満杯 → over（イベントなし）
                         counter==4 → eventDue
pass()     [MustPass]  : counter++, turn 交代, counter==4 → eventDue
fireDueEvent() [EventDue]: nextEvent 適用, lastMove 写像, counter=0, eventDue=false, nextEvent=pick()
resolveBothBlocked() [BothBlocked]:
                         パスは記録しない。nextEvent 適用, lastMove 写像, counter=0, nextEvent=pick()
                         適用後も両者合法手なし → over（FR-017）
```

## MatchSettings（プレゼンテーション層）

| フィールド | 型 | 既定 |
|-----------|----|------|
| difficulty | Weak/Normal/Strong | Normal |
| sideChoice | Black/White/Random | Black |
| playerColor | Color | 対局開始時に決定（Random は毎局再抽選） |

## MatchResult

| フィールド | 説明 |
|-----------|------|
| outcome | Win / Lose / Draw（プレイヤー視点, FR-018） |
| playerCount, cpuCount | 石数 |
| finalBoard | 終局盤面 |
