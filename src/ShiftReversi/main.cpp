// SHIFT REVERSI エントリポイント（DxLib 初期化とメインループ）
#include <algorithm>
#include <cstdio>
#include <string>

#include "App.h"
#include "DebugOptions.h"
#include "DxLib.h"
#include "Ui.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR cmdLine, _In_ int) {
#ifdef _DEBUG
    parseDebugOptions(cmdLine);
#else
    (void)cmdLine;
#endif
    const DebugOptions& dbg = debugOptions();
    SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);  // ソースの UTF-8 文字列をそのまま描画
    SetOutApplicationLogValidFlag(FALSE);
    ChangeWindowMode(TRUE);
    SetGraphMode(kScreenW, kScreenH, 32);  // FR-031
    SetMainWindowText("SHIFT REVERSI");
    SetAlwaysRunFlag(TRUE);
    SetWaitVSyncFlag(TRUE);
    if (DxLib_Init() == -1) return -1;
    SetDrawScreen(DX_SCREEN_BACK);

    {
        App app;
        LONGLONG prev = GetNowHiPerformanceCount();
#ifdef _DEBUG
        double fpsTimer = 0.0;
        int frames = 0;
        double fps = 0.0;
#endif
        double totalTime = 0.0;
        double nextCapture = 0.0;
        int captureIndex = 0;
        while (ProcessMessage() == 0 && !app.quitRequested()) {
            const LONGLONG now = GetNowHiPerformanceCount();
            // 長時間停止（ウィンドウ移動など）でアニメが飛ばないよう上限を設ける
            const double dt = std::min(static_cast<double>(now - prev) / 1'000'000.0, 0.1);
            prev = now;

            app.update(dt);
            ClearDrawScreen();
            app.draw();
#ifdef _DEBUG
            fpsTimer += dt;
            ++frames;
            if (fpsTimer >= 0.5) {
                fps = frames / fpsTimer;
                fpsTimer = 0.0;
                frames = 0;
            }
            DrawFormatString(kScreenW - 110, kScreenH - 22, GetColor(120, 255, 120), "FPS %.1f", fps);
#endif
            totalTime += dt;
            if (!dbg.captureDir.empty() && totalTime >= nextCapture) {
                char path[512];
                std::snprintf(path, sizeof(path), "%s/frame_%04d.png", dbg.captureDir.c_str(), captureIndex++);
                SaveDrawScreenToPNG(0, 0, kScreenW, kScreenH, path);
                nextCapture += dbg.captureInterval;
            }
            if (dbg.quitAfter > 0.0 && totalTime >= dbg.quitAfter) app.requestQuit();
            ScreenFlip();
            // VSync が無効な環境向けに 60FPS を上限とする
            const LONGLONG frameEnd = prev + 16'667;
            while (GetNowHiPerformanceCount() < frameEnd - 1'500) WaitTimer(1);
        }
    }

    DxLib_End();
    return 0;
}
