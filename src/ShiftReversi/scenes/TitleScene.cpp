#include "scenes/TitleScene.h"

#include <cmath>

#include "App.h"
#include "BoardView.h"
#include "DxLib.h"

using namespace shift;

namespace {
constexpr int kOptW = 170;
constexpr int kOptH = 54;
constexpr int kOptGap = 14;
constexpr int kOptX = (kScreenW - (kOptW * 3 + kOptGap * 2)) / 2;

Rect optionRect(int row, int i) { return {kOptX + i * (kOptW + kOptGap), row == 0 ? 300 : 404, kOptW, kOptH}; }
const Rect kStart{(kScreenW - 340) / 2, 500, 340, 66};
const Rect kRules{kScreenW / 2 - 230, 594, 220, 54};
const Rect kQuit{kScreenW / 2 + 10, 594, 220, 54};

constexpr Difficulty kDiffs[3] = {Difficulty::Weak, Difficulty::Normal, Difficulty::Strong};
constexpr SideChoice kSides[3] = {SideChoice::Black, SideChoice::White, SideChoice::Random};
const char* const kSideLabels[3] = {"黒（先手）", "白（後手）", "ランダム"};
}  // namespace

void TitleScene::update(double dt) {
    time_ += dt;
    const Input& in = app_.input();
    MatchSettings& s = app_.settings();
    for (int i = 0; i < 3; ++i) {
        if (ui::clicked(optionRect(0, i), in)) s.difficulty = kDiffs[i];
        if (ui::clicked(optionRect(1, i), in)) s.side = kSides[i];
    }
    if (ui::clicked(kStart, in)) app_.changeScene(SceneId::Game);
    if (ui::clicked(kRules, in)) app_.changeScene(SceneId::Rules);
    if (ui::clicked(kQuit, in)) app_.requestQuit();
}

void TitleScene::draw() {
    const Fonts& f = app_.fonts();
    const Input& in = app_.input();
    const MatchSettings& s = app_.settings();
    DrawBox(0, 0, kScreenW, kScreenH, ui::colBackground(), TRUE);

    // 背景の飾り: ゆっくり流れる石の列
    {
        const BoardView deco(0, 0, 56);
        ui::AlphaScope a(40);
        const float offset = static_cast<float>(std::fmod(time_ * 30.0, 112.0));
        for (int i = -1; i < 13; ++i) {
            const float x = static_cast<float>(i * 112) + offset;
            deco.drawStone(x, 40.0f, Cell::Black);
            deco.drawStone(x + 56.0f, 40.0f, Cell::White);
            deco.drawStone(static_cast<float>(kScreenW) - x, static_cast<float>(kScreenH) - 30.0f, Cell::White);
            deco.drawStone(static_cast<float>(kScreenW) - x - 56.0f, static_cast<float>(kScreenH) - 30.0f, Cell::Black);
        }
    }

    ui::drawTextCentered(kScreenW / 2, 92, "SHIFT REVERSI", ui::colAccent(), f.title);
    ui::drawTextCentered(kScreenW / 2, 196, "4手ごとに盤面が動く、予告付きリバーシ", ui::colText(), f.medium);

    ui::drawTextCentered(kScreenW / 2, 262, "CPU の強さ", ui::colSubText(), f.body);
    for (int i = 0; i < 3; ++i) {
        ui::drawButton(optionRect(0, i), difficultyName(kDiffs[i]), f.medium, in,
                       s.difficulty == kDiffs[i] ? ui::ButtonState::Selected : ui::ButtonState::Normal);
    }
    ui::drawTextCentered(kScreenW / 2, 366, "あなたの手番", ui::colSubText(), f.body);
    for (int i = 0; i < 3; ++i) {
        ui::drawButton(optionRect(1, i), kSideLabels[i], f.medium, in,
                       s.side == kSides[i] ? ui::ButtonState::Selected : ui::ButtonState::Normal);
    }

    ui::drawButton(kStart, "対局開始", f.large, in);
    ui::drawButton(kRules, "ルール説明", f.medium, in);
    ui::drawButton(kQuit, "終了", f.medium, in);
}
