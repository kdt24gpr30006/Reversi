#include "scenes/ResultScene.h"

#include <format>

#include "App.h"
#include "BoardView.h"
#include "DxLib.h"

using namespace shift;

namespace {
const Rect kRetry{760, 520, 220, 64};
const Rect kTitle{1000, 520, 220, 64};
}  // namespace

void ResultScene::update(double) {
    const Input& in = app_.input();
    if (ui::clicked(kRetry, in)) app_.changeScene(SceneId::Game);  // 同じ設定で再戦（ランダムは再抽選）
    if (ui::clicked(kTitle, in)) app_.changeScene(SceneId::Title);
}

void ResultScene::draw() {
    const Fonts& f = app_.fonts();
    const Input& in = app_.input();
    DrawBox(0, 0, kScreenW, kScreenH, ui::colBackground(), TRUE);

    const auto& outcome = app_.outcome();
    if (!outcome) {
        ui::drawTextCentered(kScreenW / 2, 300, "結果がありません", ui::colText(), f.large);
        ui::drawButton(kTitle, "タイトルへ", f.medium, in);
        return;
    }
    const MatchOutcome& o = *outcome;

    // 終局盤面
    const BoardView view(80, 120, 48);
    ui::drawText(80, 60, "終局時の盤面", ui::colSubText(), f.body);
    view.drawBase(f.tiny);
    view.drawStones(o.finalBoard);

    std::string verdict = "引き分け";
    unsigned int color = ui::colText();
    if (o.playerCount > o.cpuCount) {
        verdict = "あなたの勝ち！";
        color = ui::colAccent();
    } else if (o.playerCount < o.cpuCount) {
        verdict = "あなたの負け…";
        color = GetColor(140, 170, 230);
    }
    ui::drawTextCentered(990, 130, verdict, color, f.headline);

    const BoardView icon(0, 0, 52);
    const Color cpu = opponent(o.playerColor);
    const int rows[2] = {270, 350};
    for (int i = 0; i < 2; ++i) {
        const Color c = i == 0 ? o.playerColor : cpu;
        const int count = i == 0 ? o.playerCount : o.cpuCount;
        icon.drawStone(790.0f, static_cast<float>(rows[i] + 24), c);
        const std::string label = std::format("{}（{}）", i == 0 ? "あなた" : "CPU", colorName(c));
        ui::drawText(830, rows[i] + 6, label, ui::colText(), f.medium);
        const std::string n = std::format("{} 個", count);
        ui::drawText(1200 - ui::textWidth(n, f.large), rows[i], n, ui::colText(), f.large);
    }
    ui::drawText(760, 440, std::format("CPU の強さ: {}", difficultyName(o.difficulty)), ui::colSubText(), f.body);

    ui::drawButton(kRetry, "リトライ", f.medium, in);
    ui::drawButton(kTitle, "タイトルへ", f.medium, in);
}
