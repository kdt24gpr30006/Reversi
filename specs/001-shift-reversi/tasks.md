# Tasks: SHIFT REVERSI（動く盤面リバーシ）

**Input**: Design documents from `specs/001-shift-reversi/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/core-api.md, quickstart.md

**Tests**: 憲章 II によりルールの自動テストは必須。テストタスクは各ストーリーの実装前に置く。

**Organization**: ユーザーストーリー単位。

## Format: `[ID] [P?] [Story] Description`

## Path Conventions

- コア: `src/ShiftCore/include/shift/*.h`, `src/ShiftCore/src/*.cpp`
- ゲーム: `src/ShiftReversi/`
- テスト: `tests/ShiftTests/`
- ビルド: `MSBuild ShiftReversi.sln /m /p:Configuration=Release /p:Platform=x64`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: リポジトリとソリューションの骨組み

- [X] T001 `.gitignore`（third_party/, build/, .vs/, *.user, x64/）を作成 in .gitignore
- [X] T002 共通ビルド設定 src/Common.props（各 vcxproj から Import。C++20, /utf-8, /W4, /permissive-, 静的ランタイム MT/MTd, 出力 build/x64/$(Configuration)/, 中間 build/obj/$(ProjectName)/）in src/Common.props
- [X] T003 [P] 静的ライブラリプロジェクト（include パス src/ShiftCore/include, x64 Debug/Release のみ）in src/ShiftCore/ShiftCore.vcxproj
- [X] T004 [P] Windows アプリプロジェクト（SubSystem Windows, MultiByte, DxLib include/lib パス $(SolutionDir)third_party\DxLib, ShiftCore 参照）in src/ShiftReversi/ShiftReversi.vcxproj
- [X] T005 [P] コンソールテストプロジェクト（ShiftCore 参照）in tests/ShiftTests/ShiftTests.vcxproj
- [X] T006 3 プロジェクトを含むソリューション（x64 Debug/Release）in ShiftReversi.sln
- [X] T007 [P] 最小テストフレームワーク（TEST マクロ自己登録, CHECK/CHECK_EQ, 失敗数集計）in tests/ShiftTests/TestFramework.h とランナー in tests/ShiftTests/main.cpp

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: すべてのストーリーが依存するコア型と描画基盤

- [X] T008 [P] Cell/Color/opponent/Pos/kSize/toIndex/inBounds/posName in src/ShiftCore/include/shift/Types.h
- [X] T009 [P] Random（mt19937 包装、below(n) リジェクション法、chance(percent)）in src/ShiftCore/include/shift/Random.h
- [X] T010 [P] Board テスト（初期配置 E5白F5黒E6黒F6白、8方向反転、端打ち切り＝回り込み無し、非合法手、満杯判定、count）in tests/ShiftTests/BoardTests.cpp
- [X] T011 Board 実装（initial, at/set, flipsFor, legalMoves, place, count, isFull, operator==）in src/ShiftCore/include/shift/Board.h, src/ShiftCore/src/Board.cpp
- [X] T012 DxLib 初期化とメインループ（ChangeWindowMode, SetGraph 1280×720, SetUseCharCodeFormat UTF8, SetAlwaysRunFlag, ダブルバッファ, ProcessMessage ループ, 60FPS 待機）in src/ShiftReversi/main.cpp
- [X] T013 [P] マウス入力（位置, 左押下エッジ）in src/ShiftReversi/Input.h, src/ShiftReversi/Input.cpp
- [X] T014 [P] ボタン/選択トグル UI（矩形, ホバー色, 無効状態, クリック判定, 中央揃え文字）in src/ShiftReversi/Ui.h, src/ShiftReversi/Ui.cpp
- [X] T015 シーン基底とシーン管理・共有リソース（フォントハンドル, MatchSettings, 遷移要求）in src/ShiftReversi/scenes/Scene.h, src/ShiftReversi/App.h, src/ShiftReversi/App.cpp

**Checkpoint**: Release|x64 ビルド成功、ShiftTests で Board テスト全 PASS、空ウィンドウが 1280×720 で起動する

---

## Phase 3: User Story 1 - 動く盤面で CPU と 1 局対戦する (Priority: P1) 🎯 MVP

**Goal**: 予告付き盤面イベントのある対局を CPU と終局まで遊べる（暫定で起動直後に既定設定の対局開始）

**Independent Test**: 起動→対局→4 手ごとに予告どおりのイベント発動→終局判定（quickstart 手動確認 3〜8）、ShiftTests 全 PASS

### Tests for User Story 1

- [X] T016 [P] [US1] BoardEvent テスト（行右/左・列下/上スライドの回り込み、区画 4 つ×CW/CCW、4 回 CW で恒等、CW→CCW で恒等、石数不変、mapPos が apply と一致、候補 48 件、describe 文言）in tests/ShiftTests/EventTests.cpp
- [X] T017 [P] [US1] EventPicker テスト（無変化候補の除外、片種別全除外時の他種別選択、全除外時のフォールバック、同シード同結果、種別比率がほぼ 50%）in tests/ShiftTests/PickerTests.cpp
- [X] T018 [P] [US1] Game テスト（初期状態 counter0/黒番/予告あり、着手で counter++、4 手目で EventDue→fire で counter0＋新予告、パスも 1 手、パスで 4 手目到達時の発動、満杯で即終局しイベントなし、BothBlocked でパス非加算の前倒し発動、前倒し後も両者不可で終局、イベント後は反転しない、lastMove 追従）in tests/ShiftTests/GameTests.cpp
- [X] T019 [P] [US1] AI テスト（合法手のみ返す、合法手なしで nullopt、各強さで 3 秒以内、cancel で即戻る、強が予告イベントを考慮した手を選ぶ固定局面）in tests/ShiftTests/AiTests.cpp

### Implementation for User Story 1

- [X] T020 [US1] BoardEvent（kind/index/dir, apply, mapPos, allCandidates, describe 日本語）in src/ShiftCore/include/shift/BoardEvent.h, src/ShiftCore/src/BoardEvent.cpp
- [X] T021 [US1] pickEvent（FR-012/013）in src/ShiftCore/include/shift/EventPicker.h, src/ShiftCore/src/EventPicker.cpp
- [X] T022 [US1] Game（status, play, pass, fireDueEvent, resolveBothBlocked, lastMove 写像, 終局判定）in src/ShiftCore/include/shift/Game.h, src/ShiftCore/src/Game.cpp
- [X] T023 [US1] AI（弱 80/20、中 深さ3 αβ、強 反復深化αβ＋予告イベント適用・締切・cancel、評価関数）in src/ShiftCore/include/shift/Ai.h, src/ShiftCore/src/Ai.cpp
- [X] T024 [US1] 自己対戦ベンチ `--bench N [--strong-ms M]` in tests/ShiftTests/main.cpp
- [X] T025 [P] [US1] アニメーション進行率（flip 0.3s, event 0.6s, イージング）in src/ShiftReversi/Animation.h
- [X] T026 [P] [US1] CPU ワーカー（Game コピーを std::async, cancel, 結果ポーリング, 最低 0.5 秒）in src/ShiftReversi/CpuWorker.h, src/ShiftReversi/CpuWorker.cpp
- [X] T027 [US1] 盤描画（盤・格子・座標ラベル・石・合法手ハイライト・直前着手印・ホバー半透明石＋反転対象強調・予告オーバーレイ（行列帯＋矢印／区画枠＋回転矢印）・反転アニメ・スライドアニメ（SetDrawArea で回り込み）・回転アニメ・ミニ盤描画）in src/ShiftReversi/BoardView.h, src/ShiftReversi/BoardView.cpp
- [X] T028 [US1] 対局シーン（フェーズ WaitHuman/WaitCpu/AnimFlip/AnimEvent/ShowPass/ShowBothPass/Finished、サイドパネル: 手番・石数・プレイヤー色・強さ・予告文・あと N 手・思考中・パス表示）in src/ShiftReversi/scenes/GameScene.h, src/ShiftReversi/scenes/GameScene.cpp
- [X] T029 [US1] （US2 実装で置換済み）終局時は盤上に「対局終了」を表示してからリザルトへ遷移 in src/ShiftReversi/App.cpp

**Checkpoint**: 1 局を最後まで遊べる。ShiftTests 全 PASS

---

## Phase 4: User Story 2 - タイトルで設定して開始し、結果を確認して再戦 (Priority: P2)

**Goal**: タイトル → 対局 → リザルト → リトライ／タイトルの流れ

**Independent Test**: quickstart 手動確認 1, 9, 10

- [X] T030 [P] [US2] タイトルシーン（作品名、強さ 弱/中/強、手番 黒/白/ランダム、対局開始・ルール説明・終了、直前設定を保持）in src/ShiftReversi/scenes/TitleScene.h, src/ShiftReversi/scenes/TitleScene.cpp
- [X] T031 [P] [US2] リザルトシーン（勝ち/負け/引き分け、両者石数、終局盤面ミニ描画、リトライ・タイトルへ）in src/ShiftReversi/scenes/ResultScene.h, src/ShiftReversi/scenes/ResultScene.cpp
- [X] T032 [US2] 手番ランダムの毎局抽選と playerColor 決定、MatchResult 受け渡し、起動時をタイトルに変更、GameScene 終局→リザルト遷移 in src/ShiftReversi/App.cpp, src/ShiftReversi/scenes/GameScene.cpp

**Checkpoint**: クリックのみで全画面遷移が行き詰まりなく行える

---

## Phase 5: User Story 3 - ルール説明画面 (Priority: P3)

**Goal**: 図解付き 5 ページのルール説明

**Independent Test**: quickstart 手動確認 2

- [X] T033 [US3] ルール説明シーン（基本ルール／盤面イベントと予告／行・列スライドと回り込み／区画回転／パスと終局、各ページ before/after ミニ盤と矢印、ページ番号、前へ・次へ（端で無効）・タイトルへ）in src/ShiftReversi/scenes/RulesScene.h, src/ShiftReversi/scenes/RulesScene.cpp
- [X] T034 [US3] タイトルの「ルール説明」からの遷移を接続 in src/ShiftReversi/App.cpp

**Checkpoint**: 全ストーリーが独立して動作

---

## Phase 6: Polish & Cross-Cutting Concerns

- [X] T035 [P] README（概要・ルール・DxLib 取得・ビルド・テスト・ベンチ）in README.md
- [X] T036 Release/Debug 両構成で警告確認・ビルド、ShiftTests 全 PASS、`--bench 100 --strong-ms 200` で SC-002 確認、結果を記録 in specs/001-shift-reversi/quickstart.md
- [ ] T037 ゲームを起動し quickstart 手動確認 1〜10 を実施、FPS 表示（Debug のみ）で SC-004 を確認 in src/ShiftReversi/main.cpp

---

## Dependencies & Execution Order

### Phase Dependencies

- Setup (1) → Foundational (2) → US1 (3) → US2 (4) → US3 (5) → Polish (6)

### User Story Dependencies

- US1: Foundational のみに依存（MVP）
- US2: US1 の GameScene に依存（遷移接続のため）
- US3: Foundational と US1 の BoardView ミニ盤描画に依存。US2 のタイトルから遷移

### Within Each User Story

- テスト（T016〜T019）→ コア（T020→T021→T022→T023）→ プレゼンテーション（T025〜T029）

### Parallel Opportunities

- T003/T004/T005/T007、T008/T009/T010、T013/T014、T016〜T019、T025/T026、T030/T031、T035

## Parallel Example: User Story 1

```text
T016 EventTests.cpp / T017 PickerTests.cpp / T018 GameTests.cpp / T019 AiTests.cpp を同時に作成
T025 Animation.h / T026 CpuWorker.* を同時に作成
```

## Implementation Strategy

1. Phase 1〜2 完了でビルド・テスト基盤を確立
2. US1 完了で MVP（対局単体）をデモ可能に
3. US2 でゲームとしての一連の流れ、US3 でルール理解支援
4. 各チェックポイントでビルド・ShiftTests・起動確認を実施しコミット
