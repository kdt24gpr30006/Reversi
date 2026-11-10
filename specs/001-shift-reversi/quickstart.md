# Quickstart: SHIFT REVERSI の検証手順

## 前提

- Windows 11、Visual Studio 2026（C++ によるデスクトップ開発、v145 ツールセット、Windows SDK 10.0.26100）
- インターネット接続（初回の DxLib 取得時のみ）

## 0. ビルドせずに遊ぶ

`release/ShiftReversi.exe` を起動する（Windows 標準 DLL 以外への依存なし）。

## 1. DxLib の配置

リポジトリに同梱済みのため不要。取得し直す場合のみ:

```powershell
pwsh -File tools/fetch-dxlib.ps1   # 公式から取得し、必要なファイルだけに整理
```

期待: `third_party/DxLib/DxLib.h` と x64 用 `.lib` 群が存在する。

## 2. ビルド

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" `
  ShiftReversi.sln /m /p:Configuration=Release /p:Platform=x64 /v:minimal
```

期待: エラー 0 で `build/x64/Release/` に `ShiftReversi.exe` と `ShiftTests.exe` が生成される。
Visual Studio で `ShiftReversi.sln` を開き、`ShiftReversi` をスタートアップにして F5 でも可。

## 3. 自動テスト（憲章 II / SC-001, SC-003）

```powershell
build/x64/Release/ShiftTests.exe
```

期待: 全テスト PASS、終了コード 0。対象は [contracts/core-api.md](contracts/core-api.md) の Game / BoardEvent / EventPicker / AI。

## 4. CPU の強さベンチ（SC-002）

```powershell
build/x64/Release/ShiftTests.exe --bench 100 --strong-ms 200
```

期待: 「強 vs 弱」勝率 ≥ 80%、「中 vs 弱」勝率 ≥ 70%。

## 5. 手動確認（US1〜US3）

| # | 操作 | 期待 |
|---|------|------|
| 1 | 起動 | 1280×720 ウィンドウにタイトル「SHIFT REVERSI」、強さ（中）・手番（黒）が選択済み |
| 2 | ルール説明 → 次へ/前へ → タイトルへ | 全ページ遷移、先頭/末尾でボタン無効、図解あり |
| 3 | 対局開始（中・黒） | 中央 4 石、合法手ハイライト、予告と「あと 4 手」表示 |
| 4 | 合法手にホバー | 半透明石と反転対象の強調 |
| 5 | 4 手進める | 反転アニメ後に予告どおりのイベントがアニメ付きで発動、新しい予告「あと 4 手」 |
| 6 | イベントで直前着手の石が動く | 印が石に追従 |
| 7 | CPU 思考中 | 「思考中」表示、ウィンドウが固まらない、3 秒以内に着手 |
| 8 | 終局まで | リザルトに勝敗と石数、終局盤面 |
| 9 | リトライ / タイトルへ | 同設定で再戦 / タイトルで直前の設定が選択済み |
| 10 | 手番「白」「ランダム」 | CPU が先手で打つ / 割り当てた色が表示される |

## 検証記録（2026-11-11）

| 項目 | 結果 |
|------|------|
| ビルド（Debug / Release, x64, /W4） | 警告 0・エラー 0 |
| `ShiftTests.exe`（Debug / Release） | 38/38 PASS、終了コード 0 |
| SC-003 強の思考時間 | 2800ms で打ち切り（空き 96/70/40 局面、深さ 7〜10） |
| SC-002 `--bench 100 --strong-ms 200` | 強 vs 弱 100勝0分0敗（100%）、中 vs 弱 88勝2分10敗（88%） |
| 強が予告イベントを読むか | 空き 4 の終盤 40 局面で完全読みの最善値と一致（うち 8 局面はイベント考慮で最善手が変化） |
| 画面確認（Debug 起動オプションで自動対局・連番キャプチャ） | タイトル、ルール 5 ページ、対局（予告帯・矢印、思考中、直前手の印、区画回転・列スライドのアニメ）、リザルト（白番選択時は CPU が先手）、FPS 56〜62 |
| 未実施（実際のマウス操作が必要） | 手動確認 4（ホバー表示）と、各ボタンを実際にクリックしての遷移（2・9）。手順は上表のとおり |
