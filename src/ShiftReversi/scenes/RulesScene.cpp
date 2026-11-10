#include "scenes/RulesScene.h"

#include <format>
#include <vector>

#include <algorithm>

#include "App.h"
#include "BoardView.h"
#include "DebugOptions.h"
#include "DxLib.h"

using namespace shift;

namespace {
const Rect kPrev{80, 630, 200, 56};
const Rect kNext{1000, 630, 200, 56};
const Rect kBack{(kScreenW - 220) / 2, 630, 220, 56};

constexpr int kMiniCell = 22;
constexpr int kLeftX = 700;
constexpr int kRightX = 990;
constexpr int kDiagramY = 210;

struct PageText {
    const char* title;
    std::vector<const char*> lines;
};

const PageText kPages[RulesScene::kPageCount] = {
    {"1. 基本ルール",
     {"10×10 の盤で、黒（先手）と白が交互に石を置きます。", "相手の石を自分の石ではさむと、", "はさんだ石が自分の色にひっくり返ります。",
      "縦・横・ななめ、どの方向でもはさめます。", "置ける場所は盤上で光って表示されます。",
      "マウスを乗せると、ひっくり返る石が分かります。", "最後に石が多いほうの勝ちです。"}},
    {"2. 盤面イベントと予告",
     {"4 手ごとに「盤面イベント」が自動で起こり、", "石の配置が動きます。パスも 1 手と数えます。",
      "次に起こるイベントは「予告」として、", "黄色の帯と矢印で盤上に表示されます。",
      "右のパネルの「あと N 手」が 0 になると発動し、", "すぐに次のイベントが予告されます。",
      "イベントで石が動いても、石はひっくり返りません。"}},
    {"3. 行・列スライドと回り込み",
     {"1 つの行（または列）の石が、", "矢印の方向へ 1 マスずれます。",
      "盤の端からはみ出した石は、", "反対側の端に回り込みます。", "角や辺の石も安全ではありません！",
      "予告を見て、動いたあとの形を読みましょう。"}},
    {"4. 区画回転",
     {"盤は太い線で 5×5 の 4 区画に分かれています。", "どれか 1 区画が、時計回りか反時計回りに",
      "90° 回転します。", "区画の外の石は動きません。", "回転で角に入る石、角から出る石に注目！"}},
    {"5. パスと終局",
     {"置ける場所がないときは自動でパスになります。", "（パスも 1 手として数えます）",
      "両者とも置けなくなったら、予告のイベントが", "すぐに発動します（このときパスは数えず、",
      "手数はリセットされます）。", "それでも両者置けなければ対局終了です。",
      "盤がすべて埋まったときも対局終了です。", "石の多いほうが勝ち、同じなら引き分けです。"}},
};

Board fromRows(const std::vector<const char*>& rows) {
    Board b;
    for (int y = 0; y < kSize; ++y)
        for (int x = 0; x < kSize; ++x) {
            const char c = rows[static_cast<size_t>(y)][x];
            b.set({x, y}, c == 'B' ? Cell::Black : c == 'W' ? Cell::White : Cell::Empty);
        }
    return b;
}

void drawArrowBetween(int y) {
    const float x0 = static_cast<float>(kLeftX + kMiniCell * kSize + 14);
    const float x1 = static_cast<float>(kRightX - 22);
    const float yf = static_cast<float>(y);
    DrawLineAA(x0, yf, x1 - 12.0f, yf, ui::colAccent(), 4.0f);
    DrawTriangleAA(x1, yf, x1 - 16.0f, yf - 11.0f, x1 - 16.0f, yf + 11.0f, ui::colAccent(), TRUE);
}

void beforeAfter(const Board& before, const Board& after, const BoardEvent* ev, double time, int captionFont,
                 const char* leftCaption, const char* rightCaption) {
    const BoardView left(kLeftX, kDiagramY, kMiniCell);
    const BoardView right(kRightX, kDiagramY, kMiniCell);
    left.drawBase();
    if (ev) left.drawEventBand(*ev, time);
    left.drawStones(before);
    if (ev) left.drawEventArrow(*ev);
    right.drawBase();
    right.drawStones(after);
    drawArrowBetween(kDiagramY + kMiniCell * kSize / 2);
    const int capY = kDiagramY + kMiniCell * kSize + 22;
    ui::drawTextCentered(kLeftX + kMiniCell * kSize / 2, capY, leftCaption, ui::colSubText(), captionFont);
    ui::drawTextCentered(kRightX + kMiniCell * kSize / 2, capY, rightCaption, ui::colSubText(), captionFont);
}
}  // namespace

RulesScene::RulesScene(App& app) : Scene(app) {
    // Debug 検証用の開始ページ指定（一度だけ使う）
    DebugOptions& o = debugOptions();
    page_ = std::clamp(o.rulesPage, 0, kPageCount - 1);
    o.rulesPage = 0;
}

void RulesScene::update(double dt) {
    time_ += dt;
    const Input& in = app_.input();
    const auto prevState = page_ == 0 ? ui::ButtonState::Disabled : ui::ButtonState::Normal;
    const auto nextState = page_ == kPageCount - 1 ? ui::ButtonState::Disabled : ui::ButtonState::Normal;
    if (ui::clicked(kPrev, in, prevState)) --page_;
    if (ui::clicked(kNext, in, nextState)) ++page_;
    if (ui::clicked(kBack, in)) app_.changeScene(SceneId::Title);
}

void RulesScene::drawDiagram() const {
    const int cap = app_.fonts().tiny;
    switch (page_) {
    case 0: {
        Board before = Board::initial();
        Board after = before;
        after.place({4, 3}, Cell::Black);  // E4
        beforeAfter(before, after, nullptr, time_, cap, "黒が E4 に置くと…", "E5 の白がはさまれて黒に");
        break;
    }
    case 1: {
        const Board b = fromRows({
            "..........", "..........", "...B......", "...BW.....", "....WB....",
            "....BWW...", "......B...", "..........", "..........", "..........",
        });
        const BoardEvent ev{EventKind::SlideRow, 4, EventDir::Positive};
        const BoardView view(kLeftX + 120, kDiagramY - 40, 26);
        view.drawBase();
        view.drawEventBand(ev, time_);
        view.drawStones(b);
        view.drawEventArrow(ev);
        const Fonts& f = app_.fonts();
        ui::drawText(kLeftX + 120, kDiagramY + 26 * kSize - 20, "予告: 5行目を→ 右へ1マススライド", ui::colText(), f.body);
        ui::drawText(kLeftX + 120, kDiagramY + 26 * kSize + 12, "あと 2 手", ui::colAccent(), f.large);
        break;
    }
    case 2: {
        const Board before = fromRows({
            "..........", "..........", "..........", "..........", "W.BBW...BW",
            "....BW....", "..........", "..........", "..........", "..........",
        });
        const BoardEvent ev{EventKind::SlideRow, 4, EventDir::Positive};
        beforeAfter(before, ev.applied(before), &ev, time_, cap, "5行目が右へスライド", "右端の白が左端へ回り込む");
        break;
    }
    case 3: {
        const Board before = fromRows({
            "B...W.....", ".B........", "..W.......", "...W......", "W...B.....",
            "..........", "..........", "..........", "..........", "..........",
        });
        const BoardEvent ev{EventKind::Rotate, 0, EventDir::Positive};
        beforeAfter(before, ev.applied(before), &ev, time_, cap, "左上区画が時計回りに回転", "角の黒が右上へ移動");
        break;
    }
    case 4: {
        const Board before = fromRows({
            "B.........", "..W.......", "..........", "..........", "..........",
            "..........", "..........", "..........", "..........", "..........",
        });
        const BoardEvent ev{EventKind::SlideRow, 1, EventDir::Negative};
        beforeAfter(before, ev.applied(before), &ev, time_, cap, "両者とも置けない → 即発動", "黒が C3 に置けるようになった");
        break;
    }
    default:
        break;
    }
}

void RulesScene::draw() {
    const Fonts& f = app_.fonts();
    const Input& in = app_.input();
    DrawBox(0, 0, kScreenW, kScreenH, ui::colBackground(), TRUE);
    ui::drawText(80, 40, "ルール説明", ui::colSubText(), f.body);

    const PageText& p = kPages[page_];
    ui::drawText(80, 80, p.title, ui::colAccent(), f.large);
    int y = 160;
    for (const char* line : p.lines) {
        ui::drawText(80, y, line, ui::colText(), f.body);
        y += 40;
    }
    drawDiagram();

    ui::drawTextCentered(kScreenW / 2, 590, std::format("{} / {}", page_ + 1, kPageCount), ui::colSubText(), f.body);
    ui::drawButton(kPrev, "◀ 前へ", f.medium, in, page_ == 0 ? ui::ButtonState::Disabled : ui::ButtonState::Normal);
    ui::drawButton(kNext, "次へ ▶", f.medium, in,
                   page_ == kPageCount - 1 ? ui::ButtonState::Disabled : ui::ButtonState::Normal);
    ui::drawButton(kBack, "タイトルへ", f.medium, in);
}
