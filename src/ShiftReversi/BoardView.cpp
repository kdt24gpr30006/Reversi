#include "BoardView.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "DxLib.h"

using namespace shift;

namespace {
constexpr float kPi = 3.14159265f;
constexpr int kCirclePoints = 40;

bool contains(const std::vector<Pos>& v, Pos p) { return std::find(v.begin(), v.end(), p) != v.end(); }

void triangle(float tipX, float tipY, float dirX, float dirY, float size, unsigned int color) {
    // dir 方向を向いた矢じり
    const float nx = -dirY;
    const float ny = dirX;
    const float bx = tipX - dirX * size;
    const float by = tipY - dirY * size;
    DrawTriangleAA(tipX, tipY, bx + nx * size * 0.6f, by + ny * size * 0.6f, bx - nx * size * 0.6f,
                   by - ny * size * 0.6f, color, TRUE);
}

void resetDrawArea() { SetDrawArea(0, 0, kScreenW, kScreenH); }
}  // namespace

std::optional<Pos> BoardView::hit(int mx, int my) const {
    if (!rect().contains(mx, my)) return std::nullopt;
    return Pos{(mx - ox_) / cell_, (my - oy_) / cell_};
}

void BoardView::drawBase(int labelFont) const {
    const Rect r = rect();
    const int frame = std::max(4, cell_ / 6);
    DrawBox(r.x - frame, r.y - frame, r.x + r.w + frame, r.y + r.h + frame, GetColor(92, 62, 36), TRUE);
    DrawBox(r.x, r.y, r.x + r.w, r.y + r.h, GetColor(28, 120, 72), TRUE);
    const unsigned int line = GetColor(16, 70, 42);
    for (int i = 0; i <= kSize; ++i) {
        DrawLine(r.x + i * cell_, r.y, r.x + i * cell_, r.y + r.h, line);
        DrawLine(r.x, r.y + i * cell_, r.x + r.w, r.y + i * cell_, line);
    }
    // 5×5 区画の境界（区画回転の目安）
    const unsigned int border = GetColor(10, 52, 30);
    const float mid = static_cast<float>(kQuadrantSize * cell_);
    DrawLineAA(static_cast<float>(r.x) + mid, static_cast<float>(r.y), static_cast<float>(r.x) + mid,
               static_cast<float>(r.y + r.h), border, 3.0f);
    DrawLineAA(static_cast<float>(r.x), static_cast<float>(r.y) + mid, static_cast<float>(r.x + r.w),
               static_cast<float>(r.y) + mid, border, 3.0f);

    if (labelFont >= 0) {
        const unsigned int col = ui::colSubText();
        const int fh = GetFontSizeToHandle(labelFont);
        for (int i = 0; i < kSize; ++i) {
            const std::string colName(1, static_cast<char>('A' + i));
            ui::drawTextCentered(static_cast<int>(centerX(static_cast<float>(i))), r.y - frame - fh - 2, colName, col,
                                 labelFont);
            const std::string rowName = std::to_string(i + 1);
            ui::drawText(r.x - frame - 6 - ui::textWidth(rowName, labelFont),
                         static_cast<int>(centerY(static_cast<float>(i))) - fh / 2, rowName, col, labelFont);
        }
    }
}

void BoardView::drawStone(float px, float py, Color color, float scaleX, float scale) const {
    const float r = static_cast<float>(cell_) * 0.42f * scale;
    const float rx = std::max(0.5f, r * scaleX);
    if (color == Cell::Black) {
        DrawOvalAA(px, py + 1.5f, rx, r, kCirclePoints, GetColor(8, 8, 8), TRUE);
        DrawOvalAA(px, py, rx, r, kCirclePoints, GetColor(30, 30, 34), TRUE);
        DrawOvalAA(px - rx * 0.25f, py - r * 0.3f, rx * 0.35f, r * 0.25f, 20, GetColor(70, 70, 78), TRUE);
    } else {
        DrawOvalAA(px, py + 1.5f, rx, r, kCirclePoints, GetColor(120, 120, 120), TRUE);
        DrawOvalAA(px, py, rx, r, kCirclePoints, GetColor(238, 238, 232), TRUE);
        DrawOvalAA(px, py, rx, r, kCirclePoints, GetColor(180, 180, 175), FALSE, 1.0f);
    }
}

void BoardView::drawStones(const Board& board) const {
    for (int y = 0; y < kSize; ++y) {
        for (int x = 0; x < kSize; ++x) {
            const Cell c = board.at(x, y);
            if (c != Cell::Empty) drawStone(centerX(static_cast<float>(x)), centerY(static_cast<float>(y)), c);
        }
    }
}

void BoardView::drawLegalMoves(const std::vector<Pos>& moves, double time) const {
    const int alpha = 120 + static_cast<int>(60.0 * std::sin(time * 4.0));
    ui::AlphaScope a(alpha);
    for (const Pos p : moves) {
        DrawCircleAA(centerX(static_cast<float>(p.x)), centerY(static_cast<float>(p.y)),
                     static_cast<float>(cell_) * 0.13f, 24, GetColor(255, 230, 120), TRUE);
    }
}

void BoardView::drawLastMove(Pos p) const {
    DrawCircleAA(centerX(static_cast<float>(p.x)), centerY(static_cast<float>(p.y)),
                 static_cast<float>(cell_) * 0.09f, 16, GetColor(230, 60, 60), TRUE);
}

void BoardView::drawHover(Pos p, Color color, const std::vector<Pos>& flips) const {
    {
        ui::AlphaScope a(130);
        drawStone(centerX(static_cast<float>(p.x)), centerY(static_cast<float>(p.y)), color);
    }
    for (const Pos f : flips) {
        DrawCircleAA(centerX(static_cast<float>(f.x)), centerY(static_cast<float>(f.y)),
                     static_cast<float>(cell_) * 0.46f, kCirclePoints, GetColor(255, 140, 40), FALSE, 3.0f);
    }
}

void BoardView::drawRowColArrow(const BoardEvent& e, unsigned int color, float thickness) const {
    const Rect r = rect();
    const float sign = e.dir == EventDir::Positive ? 1.0f : -1.0f;
    const float tri = static_cast<float>(cell_) * 0.4f;
    const float gap = static_cast<float>(cell_) * 0.15f;
    if (e.kind == EventKind::SlideRow) {
        const float y = centerY(static_cast<float>(e.index));
        const float x0 = static_cast<float>(r.x) + static_cast<float>(cell_) * 0.3f;
        const float x1 = static_cast<float>(r.x + r.w) - static_cast<float>(cell_) * 0.3f;
        const float from = sign > 0 ? x0 : x1;
        const float to = sign > 0 ? x1 : x0;
        DrawLineAA(from, y, to - sign * tri, y, color, thickness);
        triangle(to, y, sign, 0.0f, tri, color);
        // 盤外: 出ていく側と入ってくる側（回り込み）
        const float outX = sign > 0 ? static_cast<float>(r.x + r.w) + gap + tri : static_cast<float>(r.x) - gap - tri;
        const float inX = sign > 0 ? static_cast<float>(r.x) - gap : static_cast<float>(r.x + r.w) + gap;
        triangle(outX, y, sign, 0.0f, tri * 0.8f, color);
        triangle(inX, y, sign, 0.0f, tri * 0.8f, color);
    } else {
        const float x = centerX(static_cast<float>(e.index));
        const float y0 = static_cast<float>(r.y) + static_cast<float>(cell_) * 0.3f;
        const float y1 = static_cast<float>(r.y + r.h) - static_cast<float>(cell_) * 0.3f;
        const float from = sign > 0 ? y0 : y1;
        const float to = sign > 0 ? y1 : y0;
        DrawLineAA(x, from, x, to - sign * tri, color, thickness);
        triangle(x, to, 0.0f, sign, tri, color);
        const float outY = sign > 0 ? static_cast<float>(r.y + r.h) + gap + tri : static_cast<float>(r.y) - gap - tri;
        const float inY = sign > 0 ? static_cast<float>(r.y) - gap : static_cast<float>(r.y + r.h) + gap;
        triangle(x, outY, 0.0f, sign, tri * 0.8f, color);
        triangle(x, inY, 0.0f, sign, tri * 0.8f, color);
    }
}

void BoardView::drawRotateArrow(const BoardEvent& e, unsigned int color, float thickness) const {
    const Pos o = quadrantOrigin(e.index);
    const float cx = static_cast<float>(ox_) + (static_cast<float>(o.x) + 2.5f) * static_cast<float>(cell_);
    const float cy = static_cast<float>(oy_) + (static_cast<float>(o.y) + 2.5f) * static_cast<float>(cell_);
    const float radius = static_cast<float>(cell_) * 1.6f;
    const float sign = e.dir == EventDir::Positive ? 1.0f : -1.0f;  // 画面座標で角度増加 = 時計回り
    const float start = -kPi / 2.0f;
    const float sweep = kPi * 1.5f * sign;
    constexpr int kSeg = 36;
    for (int i = 0; i < kSeg; ++i) {
        const float a0 = start + sweep * static_cast<float>(i) / kSeg;
        const float a1 = start + sweep * static_cast<float>(i + 1) / kSeg;
        DrawLineAA(cx + radius * std::cos(a0), cy + radius * std::sin(a0), cx + radius * std::cos(a1),
                   cy + radius * std::sin(a1), color, thickness);
    }
    const float end = start + sweep;
    const float ex = cx + radius * std::cos(end);
    const float ey = cy + radius * std::sin(end);
    const float tx = -std::sin(end) * sign;
    const float ty = std::cos(end) * sign;
    const float tri = static_cast<float>(cell_) * 0.45f;
    triangle(ex + tx * tri * 0.6f, ey + ty * tri * 0.6f, tx, ty, tri, color);
}

void BoardView::drawEventBand(const BoardEvent& e, double time, bool strong) const {
    const int pulse = static_cast<int>(30.0 * std::sin(time * 3.0));
    const unsigned int band = strong ? GetColor(255, 90, 60) : GetColor(255, 220, 80);
    {
        ui::AlphaScope a((strong ? 130 : 90) + pulse);
        if (e.kind == EventKind::SlideRow) {
            DrawBox(ox_, oy_ + e.index * cell_, ox_ + kSize * cell_, oy_ + (e.index + 1) * cell_, band, TRUE);
        } else if (e.kind == EventKind::SlideCol) {
            DrawBox(ox_ + e.index * cell_, oy_, ox_ + (e.index + 1) * cell_, oy_ + kSize * cell_, band, TRUE);
        } else {
            const Pos o = quadrantOrigin(e.index);
            DrawBox(ox_ + o.x * cell_, oy_ + o.y * cell_, ox_ + (o.x + kQuadrantSize) * cell_,
                    oy_ + (o.y + kQuadrantSize) * cell_, band, TRUE);
        }
    }
}

void BoardView::drawEventArrow(const BoardEvent& e, bool strong) const {
    const unsigned int arrow = strong ? GetColor(255, 120, 80) : GetColor(255, 225, 110);
    const float thick = std::max(2.0f, static_cast<float>(cell_) * 0.07f);
    if (e.kind == EventKind::Rotate) {
        const Pos o = quadrantOrigin(e.index);
        DrawBoxAA(static_cast<float>(ox_ + o.x * cell_), static_cast<float>(oy_ + o.y * cell_),
                  static_cast<float>(ox_ + (o.x + kQuadrantSize) * cell_),
                  static_cast<float>(oy_ + (o.y + kQuadrantSize) * cell_), arrow, FALSE, thick);
        ui::AlphaScope a(190);
        drawRotateArrow(e, arrow, thick);
    } else {
        ui::AlphaScope a(190);
        drawRowColArrow(e, arrow, thick);
    }
}

void BoardView::drawFlipAnim(const Board& after, Pos placed, const std::vector<Pos>& flipped, Color mover,
                             double t) const {
    const float tf = static_cast<float>(t);
    for (int y = 0; y < kSize; ++y) {
        for (int x = 0; x < kSize; ++x) {
            const Pos p{x, y};
            const Cell c = after.at(p);
            if (c == Cell::Empty) continue;
            const float px = centerX(static_cast<float>(x));
            const float py = centerY(static_cast<float>(y));
            if (p == placed) {
                drawStone(px, py, c, 1.0f, std::min(1.0f, 0.6f + tf));
            } else if (contains(flipped, p)) {
                const float sx = std::fabs(std::cos(kPi * tf));
                drawStone(px, py, tf < 0.5f ? opponent(mover) : mover, sx);
            } else {
                drawStone(px, py, c);
            }
        }
    }
}

void BoardView::drawEventAnim(const Board& before, const BoardEvent& e, double t) const {
    const float tf = static_cast<float>(t);
    // 対象範囲を強調
    {
        ui::AlphaScope a(80);
        const unsigned int band = GetColor(255, 160, 60);
        if (e.kind == EventKind::SlideRow) {
            DrawBox(ox_, oy_ + e.index * cell_, ox_ + kSize * cell_, oy_ + (e.index + 1) * cell_, band, TRUE);
        } else if (e.kind == EventKind::SlideCol) {
            DrawBox(ox_ + e.index * cell_, oy_, ox_ + (e.index + 1) * cell_, oy_ + kSize * cell_, band, TRUE);
        } else {
            const Pos o = quadrantOrigin(e.index);
            DrawBox(ox_ + o.x * cell_, oy_ + o.y * cell_, ox_ + (o.x + kQuadrantSize) * cell_,
                    oy_ + (o.y + kQuadrantSize) * cell_, band, TRUE);
        }
    }
    // 対象外の石
    for (int i = 0; i < kCells; ++i) {
        const Pos p = fromIndex(i);
        const Cell c = before.at(p);
        if (c != Cell::Empty && !e.affects(p)) drawStone(centerX(static_cast<float>(p.x)), centerY(static_cast<float>(p.y)), c);
    }
    const float sign = e.dir == EventDir::Positive ? 1.0f : -1.0f;
    const Rect r = rect();
    if (e.kind == EventKind::Rotate) {
        const Pos o = quadrantOrigin(e.index);
        const float cx = static_cast<float>(ox_) + (static_cast<float>(o.x) + 2.5f) * static_cast<float>(cell_);
        const float cy = static_cast<float>(oy_) + (static_cast<float>(o.y) + 2.5f) * static_cast<float>(cell_);
        const float ang = sign * tf * kPi / 2.0f;
        const float cs = std::cos(ang);
        const float sn = std::sin(ang);
        SetDrawArea(r.x, r.y, r.x + r.w, r.y + r.h);  // 回転中に角の石が盤外へはみ出さないように
        for (int i = 0; i < kCells; ++i) {
            const Pos p = fromIndex(i);
            const Cell c = before.at(p);
            if (c == Cell::Empty || !e.affects(p)) continue;
            const float dx = centerX(static_cast<float>(p.x)) - cx;
            const float dy = centerY(static_cast<float>(p.y)) - cy;
            drawStone(cx + dx * cs - dy * sn, cy + dx * sn + dy * cs, c);
        }
        resetDrawArea();
        return;
    }
    // スライド: 盤内にクリップし、反対側からの回り込みも描く
    SetDrawArea(r.x, r.y, r.x + r.w, r.y + r.h);
    const float shift = sign * tf * static_cast<float>(cell_);
    const float wrap = -sign * static_cast<float>(kSize * cell_);
    for (int i = 0; i < kCells; ++i) {
        const Pos p = fromIndex(i);
        const Cell c = before.at(p);
        if (c == Cell::Empty || !e.affects(p)) continue;
        const float px = centerX(static_cast<float>(p.x));
        const float py = centerY(static_cast<float>(p.y));
        if (e.kind == EventKind::SlideRow) {
            drawStone(px + shift, py, c);
            drawStone(px + shift + wrap, py, c);
        } else {
            drawStone(px, py + shift, c);
            drawStone(px, py + shift + wrap, c);
        }
    }
    resetDrawArea();
}
