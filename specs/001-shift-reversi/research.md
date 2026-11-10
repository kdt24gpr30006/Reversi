# Research: SHIFT REVERSI

## R1. DxLib の入手と版

- **Decision**: 公式配布の `DxLib_VC3_25a.zip`（Ver3.25a, VisualStudio(C++)用, 約206MB）を
  `tools/fetch-dxlib.ps1` で取得し、`tools/prune-dxlib.ps1` で本プロジェクトに必要なヘッダ 5 個と
  x64 / マルチバイト / MT・MTd 用 lib 40 個（約 57MB、最大 13MB/ファイル）に絞って `third_party/DxLib/` に同梱する。
- **Rationale**: 提出がリポジトリ単位のため、クローン直後にビルドできる必要がある（2026-11-11 方針変更）。
  同梱物の `help/dxlicense.html` に「lib・ソースファイルを配布する場合は、DxLib が使用するライブラリと
  DxLib 自体の著作権表記を配布物の文書に含める」とあり、再配布は条件付きで認められている → `THIRD_PARTY_NOTICES.txt`。
- **Alternatives**: git 管理外＋取得スクリプト（クローン後すぐビルドできないため変更）、
  ビルド時の自動ダウンロード（ネットワーク依存のため却下）、DxLibMake でソースビルド（不要な複雑さで却下）。

## R2. ツールチェーン

- **Decision**: Visual Studio 2026 Community（`C:\Program Files\Microsoft Visual Studio\18\Community`）、
  PlatformToolset **v145**（MSVC 14.50）、Windows SDK 10.0.26100.0、`/std:c++20`、x64 のみ。
  ソリューションは互換性の高い `.sln` 形式。
- **Rationale**: ローカル環境で実在を確認済み。`.sln` は MSBuild コマンドラインからもそのままビルドできる。
- **Alternatives**: `.slnx`（VS2026 では利用可能だが古い環境で開けないため見送り）、CMake（依存追加となるため却下）。

## R3. ランタイムと文字コード

- **Decision**: 全プロジェクトで静的ランタイム（Release `/MT`、Debug `/MTd`）。ソースは UTF-8 で `/utf-8`、
  文字セットは「マルチバイト」。DxLib は `DxLib_Init` 前に `SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8)` を
  呼び、UTF-8 文字列を直接描画する。
- **Rationale**: DxLib の VC 用ライブラリは静的ランタイム版が標準。UTF-8 統一で日本語文字列をソースに直書きできる。
- **Alternatives**: Unicode ビルド + `L""` 文字列（コア側との文字列受け渡しが煩雑で却下）。

## R4. 盤面表現

- **Decision**: `std::array<Cell, 100>`（Empty/Black/White）。合法手判定・反転は 8 方向走査。
- **Rationale**: 10×10 は 64bit ビットボードに収まらない。100 マス配列でも強 CPU の数手〜十数万ノード/秒は
  確保でき、スライド・回転の実装とテストが単純（憲章 IV）。
- **Alternatives**: 128bit ビットボード（MSVC に `__int128` が無く、回り込みシフトも複雑で却下）。

## R5. 乱数と再現性

- **Decision**: コア内に `Random`（`std::mt19937` 包装、`uint32_t below(n)` はリジェクション法で自前実装）。
  シードはコンストラクタ注入。ゲーム本体は起動時刻からシードを生成。
- **Rationale**: `std::uniform_int_distribution` は実装依存のため、テストの期待値を固定できるよう自前で区間変換する（FR-014）。

## R6. 進行 API の粒度（アニメーションとの両立）

- **Decision**: コアの `Game` は「1 ステップずつ進める」API（`play` / `pass` / `fireDueEvent` /
  `resolveBothBlocked`）を提供し、各ステップは結果（反転したマス、適用したイベント、座標写像）を返す。
  プレゼンテーション層は結果を受け取ってアニメーションし、終了後に次ステップを呼ぶ。
- **Rationale**: 「4 手目の反転アニメ完了後にイベント発動」（FR-006）をコアの状態と表示のずれ無しに実現でき、
  表示状態を確定状態から導出できる（憲章 V）。

## R7. CPU 思考

- **Decision**:
  - 弱: 80% 一様ランダム、20% 反転数最大（同数はランダム）。
  - 中: 位置評価＋機動力の評価関数で深さ 3 のαβ。イベントは無視。
  - 強: 反復深化αβ（深さ 1 から増加）、締切 2.8 秒（3 秒上限に余裕）。探索内で手数カウンタを進め、
    4 手目到達時に予告イベントを適用。以降の未知イベントは発生しない扱い。残り空きマス 12 以下で石差評価。
  - 位置評価: 回り込みで端・角が固定されないため、通常リバーシより角の重みを抑えた表を用いる。
- **Threading**: ゲーム本体は対局状態のコピーを `std::async` に渡し、毎フレーム `future.wait_for(0)` で完了確認。
  キャンセル用に `std::atomic<bool>`。
- **Rationale**: FR-019〜021、SC-002/003。

## R8. 自動テスト

- **Decision**: `ShiftTests` コンソール exe に自作の最小アサーション（`CHECK(expr)`）とテスト登録マクロ。
  失敗時は非ゼロ終了。`--bench N [--strong-ms M]` で AI 自己対戦ベンチ（SC-002）を別モードとして実行。
- **Rationale**: 外部依存なし（憲章 IV）、MSBuild 後に exe 実行だけで検証できる（憲章 II）。
- **Alternatives**: Microsoft C++ Unit Test Framework（VS 依存が強くコマンドライン実行が煩雑）、GoogleTest（依存追加）。
- **Note**: 強 CPU の本番 3 秒設定で 100 局は数時間かかるため、ベンチでは強の締切を短縮（既定 200ms）して
  相対的な強さを検証する。思考時間上限（SC-003）は別テストで本番設定の局面を数件計測する。

## R9. 描画とアニメーション

- **Decision**: 盤 640×640（1 マス 64px）を左、サイドパネルを右に配置。スライドは対象行の石を
  `t×64px` だけずらし、`SetDrawArea` で盤内にクリップして反対側からの回り込みを描く。区画回転は区画中心を軸に
  石の中心座標を `t×90°` 回転して描く。反転は円の横幅を cos で縮めて色を切り替える。
  フォントは `CreateFontToHandle`（MS ゴシック系）。
- **Rationale**: 画像素材不使用（FR-030）で全演出を図形のみで実現できる。
