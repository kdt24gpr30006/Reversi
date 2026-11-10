# SHIFT REVERSI

**4手ごとに盤面が動く、予告付きリバーシ。**
AI（Claude Code + GitHub Spec Kit）を使って、要件定義から実装・テストまで行った新しいオセロです。

## すぐに遊ぶ

1. このリポジトリをクローン（または ZIP でダウンロードして展開）
2. **`release/ShiftReversi.exe` をダブルクリック**

インストールは不要です（Windows 10/11 x64。追加のランタイムも不要）。
操作はすべてマウスの左クリックです。

## 何が新しいのか

通常のリバーシでは、角や辺に置いた石は二度と動かない「安全な石」になります。
SHIFT REVERSI では **盤面そのものが動く** ため、その前提が崩れます。

- **4手ごと（パスも1手と数える）に「盤面イベント」が自動で起こります。**
  - **行・列スライド**：1つの行または列が1マスずれます。盤の端からはみ出た石は**反対側の端へ回り込みます**。
  - **区画回転**：盤を 5×5 に4分割した区画（太線）のうち1つが、90° 回転します。
- 次に起こるイベントは**予告**として、盤上（黄色の帯と矢印）と右パネル（「あと N 手」）に常に表示されます。
  「今どこに置くか」だけでなく、**「動いたあとにどうなるか」** を読む戦略性が生まれます。
- イベントで石が動いて挟まれた形になっても、石はひっくり返りません（反転は着手時のみ）。
- 両者とも置けなくなったら、予告中のイベントがすぐに発動します（このパスは数えず、手数はリセット）。
  それでも置けなければ終局です。盤が埋まったときも終局です。石の多いほうの勝ちです。

## 画面

| 画面 | 内容 |
|------|------|
| タイトル | CPU の強さ（弱・中・強）と手番（黒＝先手／白＝後手／ランダム）を選んで対局開始 |
| ルール説明 | 5ページ。盤面イベントを図解で説明 |
| 対局 | 10×10 盤。置ける場所の表示、ホバーで反転する石を表示、予告、アニメーション |
| リザルト | 勝敗・石数・終局盤面。リトライ／タイトルへ |

## AI の活用

### 1. 開発プロセス：AI による仕様駆動開発

AI コーディングエージェント **Claude Code** と、仕様駆動開発ツール **GitHub Spec Kit** を使って開発しました。

| 段階 | 内容 | 成果物 |
|------|------|--------|
| インタビュー | AI が作者に選択式の質問を繰り返し、要件が明確になるまで詰めた（コンセプト、盤サイズ、イベントの頻度・種類・予告範囲、パスの扱い、CPU の強さ、画面構成など） | — |
| constitution | 開発原則（ロジック分離、ルールの自動テスト必須、仕様外の機能を追加しない、など） | `.specify/memory/constitution.md` |
| specify / clarify | 機能要件 31 項目、受け入れシナリオ、エッジケース、成功基準。曖昧点を追加質問で解消 | `specs/001-shift-reversi/spec.md` |
| plan | 技術調査・データモデル・API 契約・検証手順 | `plan.md`, `research.md`, `data-model.md`, `contracts/`, `quickstart.md` |
| tasks / implement | 37 タスクに分解し、テスト → 実装の順で実装 | `tasks.md`, `src/`, `tests/` |

要件（何を作るか）はインタビューで作者が決め、仕様化・設計・実装・テストは AI が Spec Kit の手順に沿って行いました。

### 2. ゲーム内の AI：予告を読む CPU

| 強さ | 思考方法 |
|------|----------|
| 弱 | 80% はランダム、20% は最も多く返せる手 |
| 中 | マスの位置評価＋機動力（打てる手の数）で 3 手先まで αβ 探索。盤面イベントは考えない |
| 強 | **予告中の盤面イベントを探索に組み込んだ** 反復深化 αβ 探索（1手 最大 3 秒、別スレッドで思考） |

回り込みで角や辺が「安全」でなくなるため、位置評価は通常のリバーシより角の重みを抑えています。

**検証結果**（`ShiftTests.exe`）

- 自己対戦 100 局：強 vs 弱 **100%**、中 vs 弱 **88%** の勝率
- 空き 4 マスの終盤 40 局面で、強 CPU の手が「予告イベント込みで全手を読み切った最善手」と一致
  （うち 8 局面は、イベントを考慮するかどうかで最善手が変わる局面）

## ソースからビルドする

Visual Studio 2026（「C++ によるデスクトップ開発」）で **`ShiftReversi.sln` を開いて F5**。
DxLib はリポジトリに同梱しているため、追加のセットアップは不要です。
（Visual Studio 2022 で開いた場合はツールセット v143 を使う設定にしていますが、VS2022 での動作は未確認です）

コマンドラインの場合:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" `
  ShiftReversi.sln /m /p:Configuration=Release /p:Platform=x64
build/x64/Release/ShiftReversi.exe
```

### テスト

```powershell
build/x64/Release/ShiftTests.exe                               # ルール・AI の自動テスト 38 件（失敗時は終了コード 1）
build/x64/Release/ShiftTests.exe --bench 100 --strong-ms 200   # CPU 強さベンチ（強/中 vs 弱）
```

### 配布用 exe の更新

```powershell
pwsh -File tools/make-release.ps1   # Release ビルド → テスト → release/ を更新
```

## 構成

| パス | 内容 |
|------|------|
| `release/` | ビルド済みのゲーム本体と著作権表記 |
| `src/ShiftCore/` | ルール・盤面イベント・抽選・CPU（DxLib に依存しない静的ライブラリ） |
| `src/ShiftReversi/` | DxLib による画面・アニメーション・入力 |
| `tests/ShiftTests/` | 自動テストとベンチ |
| `third_party/DxLib/` | DxLib Ver3.25a（このプロジェクトのビルドに必要なヘッダと x64 用 lib のみ） |
| `specs/001-shift-reversi/` | Spec Kit の仕様・計画・タスク |
| `.specify/`, `.claude/` | Spec Kit の設定とテンプレート、AI 用コマンド |
| `tools/` | DxLib の取得・整理、配布用 exe の作成 |

Debug ビルドに限り、`ShiftReversi.exe --scene=game --autoplay --capture=DIR --capture-interval=0.5 --quit-after=60`
のように、開始画面の指定・自動対局・画面の連番保存ができます（検証用。Release ビルドでは無効）。

## ライセンス表記

本ソフトウェアは [ＤＸライブラリ](https://dxlib.xsrv.jp/)（DX Library Copyright (C) 2001-2026 Takumi Yamada.）を使用しています。
ＤＸライブラリおよび同ライブラリが使用しているライブラリの著作権表記は
[`THIRD_PARTY_NOTICES.txt`](THIRD_PARTY_NOTICES.txt) を参照してください。
