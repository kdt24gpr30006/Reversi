#pragma once
#include <optional>
#include <vector>

#include "Ui.h"
#include "shift/BoardEvent.h"

// 盤の描画。対局用の大きな盤と、ルール説明・リザルト用のミニ盤の両方に使う。
// 表示はコアの確定状態（Board）とステップ結果から導出する（憲章 V）。
class BoardView {
public:
    BoardView(int originX, int originY, int cellSize) : ox_(originX), oy_(originY), cell_(cellSize) {}

    Rect rect() const { return {ox_, oy_, cell_ * shift::kSize, cell_ * shift::kSize}; }
    float centerX(float x) const { return static_cast<float>(ox_) + (x + 0.5f) * static_cast<float>(cell_); }
    float centerY(float y) const { return static_cast<float>(oy_) + (y + 0.5f) * static_cast<float>(cell_); }
    std::optional<shift::Pos> hit(int mx, int my) const;

    // 盤面（緑の盤・格子・区画境界・座標ラベル）
    void drawBase(int labelFont = -1) const;
    void drawStone(float px, float py, shift::Color color, float scaleX = 1.0f, float scale = 1.0f) const;
    void drawStones(const shift::Board& board) const;

    void drawLegalMoves(const std::vector<shift::Pos>& moves, double time) const;
    void drawLastMove(shift::Pos p) const;
    void drawHover(shift::Pos p, shift::Color color, const std::vector<shift::Pos>& flips) const;
    // 予告オーバーレイ（FR-015）。帯は石の下、矢印は石の上に描く。
    // time はパルス表現用、strong は発動直前の強調。
    void drawEventBand(const shift::BoardEvent& e, double time, bool strong = false) const;
    void drawEventArrow(const shift::BoardEvent& e, bool strong = false) const;

    // 着手アニメーション: after は着手後の盤
    void drawFlipAnim(const shift::Board& after, shift::Pos placed, const std::vector<shift::Pos>& flipped,
                      shift::Color mover, double t) const;
    // 盤面イベントアニメーション: before は発動前の盤、t は 0..1
    void drawEventAnim(const shift::Board& before, const shift::BoardEvent& e, double t) const;

private:
    void drawRowColArrow(const shift::BoardEvent& e, unsigned int color, float thickness) const;
    void drawRotateArrow(const shift::BoardEvent& e, unsigned int color, float thickness) const;

    int ox_;
    int oy_;
    int cell_;
};
