# Implementation Plan: SHIFT REVERSI（動く盤面リバーシ）

**Branch**: `001-shift-reversi` | **Date**: 2026-11-10 | **Spec**: [spec.md](spec.md)

**Input**: Feature specification from `specs/001-shift-reversi/spec.md`

## Summary

10×10 盤で 4 手ごとに予告付きの盤面イベント（行・列スライド／5×5 区画回転）が起きるリバーシを、
DxLib + Visual Studio 2026（C++20, x64）で実装する。ルール・イベント抽選・CPU を DxLib 非依存の静的ライブラリ
`ShiftCore` に分離し、ステップ単位の進行 API（[contracts/core-api.md](contracts/core-api.md)）で
DxLib 側のシーン／アニメーションと連携させる。ルールはテスト exe `ShiftTests` で自動検証する。

## Technical Context

**Language/Version**: C++20（MSVC 14.50, PlatformToolset v145, `/std:c++20 /utf-8 /W4`）

**Primary Dependencies**: DxLib Ver3.25a（VisualStudio(C++)用, `third_party/DxLib`、ヘッダの `#pragma comment` で自動リンク）、C++ 標準ライブラリ

**Storage**: N/A（保存なし）

**Testing**: `ShiftTests` コンソール exe（自作最小アサーション、非ゼロ終了で失敗）＋ `--bench` 自己対戦

**Target Platform**: Windows 10/11 x64、Windows SDK 10.0.26100.0

**Project Type**: desktop-app（ゲーム）＋ 静的ライブラリ ＋ テスト exe

**Performance Goals**: 60 FPS（SC-004 は 55 以上）、CPU 1 手 ≤ 3 秒（強は締切 2.8 秒）

**Constraints**: 1280×720 ウィンドウ、マウスのみ、画像・音素材なし、外部依存は DxLib のみ、メインスレッド 1 フレーム < 16ms

**Scale/Scope**: 画面 4 種、コア約 1,000 行、プレゼンテーション約 1,500 行

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| 原則 | 適合方法 | 判定 |
|------|---------|------|
| I. ロジック分離 | `ShiftCore` は DxLib を include しない。プロジェクト設定でも DxLib のパスを ShiftCore に渡さない。乱数はシード注入 | ✅ |
| II. 自動テスト必須 | `ShiftTests` が全ルール項目をカバー、失敗で exit 1。品質ゲートに組み込む | ✅ |
| III. 仕様忠実 | 実装範囲は FR-001〜031 と Clarifications のみ。Undo・ポーズ・音は実装しない | ✅ |
| IV. 単純さ | 依存は DxLib のみ、素材なし、配列盤面、継承は Scene のみ | ✅ |
| V. 応答性 | CPU は `std::async` で別スレッド、毎フレーム非ブロッキング確認。表示は Game の確定状態＋ステップ結果から導出 | ✅ |
| VI. 日本語 | UI・ドキュメント日本語、識別子英語、UTF-8 描画 | ✅ |

Post-design 再確認（Phase 1 後）: data-model / contracts は上記に適合。違反なし → Complexity Tracking 不要。

## Project Structure

### Documentation (this feature)

```text
specs/001-shift-reversi/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/core-api.md
├── checklists/requirements.md
└── tasks.md             # /speckit-tasks で作成
```

### Source Code (repository root)

```text
ShiftReversi.sln
src/Common.props               # 共通: C++20, /utf-8, /W4, 静的ランタイム, 出力先 build/（各 vcxproj が Import）
.gitignore                     # third_party/, build/, .vs/ など
README.md                      # 取得・ビルド・遊び方
tools/fetch-dxlib.ps1
third_party/DxLib/             # git 管理外
src/
├── ShiftCore/                 # 静的ライブラリ（DxLib 非依存）
│   ├── ShiftCore.vcxproj
│   ├── include/shift/{Types,Board,BoardEvent,Random,EventPicker,Game,Ai}.h
│   └── src/{Board,BoardEvent,EventPicker,Game,Ai}.cpp
└── ShiftReversi/              # DxLib Windows アプリ
    ├── ShiftReversi.vcxproj
    ├── main.cpp               # WinMain, DxLib 初期化, メインループ
    ├── App.h/.cpp             # シーン管理, 共有設定, フォント
    ├── Input.h/.cpp           # マウス状態（押下エッジ）
    ├── Ui.h/.cpp              # ボタン・トグル描画とヒット判定
    ├── BoardView.h/.cpp       # 盤・石・ハイライト・予告オーバーレイ・アニメ描画
    ├── Animation.h            # 反転/スライド/回転の進行率管理
    ├── CpuWorker.h/.cpp       # std::async による CPU 思考
    ├── DebugOptions.h         # 検証用起動オプション（Debug ビルドのみ解析、Release の挙動は不変）
    └── scenes/{Scene.h, TitleScene, RulesScene, GameScene, ResultScene}.{h,cpp}
tests/
└── ShiftTests/
    ├── ShiftTests.vcxproj
    ├── TestFramework.h
    ├── main.cpp               # ランナー, --bench
    └── {BoardTests,EventTests,PickerTests,GameTests,AiTests}.cpp
```

**Structure Decision**: 3 プロジェクト構成（憲章 技術的制約）。ShiftReversi と ShiftTests は ShiftCore を
プロジェクト参照し、`src/ShiftCore/include` をインクルードパスに追加。DxLib のパスは ShiftReversi のみに設定。
ビルド出力は `build/x64/<Config>/`。

## 主要設計

- **進行**: GameScene は内部フェーズ `WaitHuman / WaitCpu / AnimFlip / AnimEvent / ShowPass / ShowBothPass / Finished`
  を持ち、`Game::status()` に応じて遷移。4 手目は `AnimFlip` 終了後に `fireDueEvent()` → `AnimEvent`（FR-006）。
- **アニメーション描画**: AnimFlip は着手後の盤＋反転マスを幅縮小で描画。AnimEvent は `EventResult::before` の盤を
  `BoardEvent` に従い補間描画し、終了後は `Game::board()` を描く。
- **CPU**: 手番開始時に Game をコピーして `std::async`。完了かつ 0.5 秒経過（FR-021）で `play`。
- **ルール説明**: 5 ページ。各ページの図は BoardView のミニ盤描画を再利用して before/after と矢印を描く。

## Complexity Tracking

違反なし。
