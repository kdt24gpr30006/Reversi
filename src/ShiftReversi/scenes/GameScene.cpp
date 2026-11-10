#include "scenes/GameScene.h"

#include <algorithm>
#include <cmath>
#include <format>

#include "App.h"
#include "DebugOptions.h"
#include "DxLib.h"

using namespace shift;

namespace {
shift::Color resolvePlayerColor(SideChoice side, Random& rng) {
    switch (side) {
    case SideChoice::Black: return Cell::Black;
    case SideChoice::White: return Cell::White;
    case SideChoice::Random: return rng.chance(50) ? Cell::Black : Cell::White;
    }
    return Cell::Black;
}
}  // namespace

GameScene::GameScene(App& app)
    : Scene(app),
      seedRng_(app.newSeed()),
      player_(resolvePlayerColor(app.settings().side, seedRng_)),
      difficulty_(app.settings().difficulty),
      game_(seedRng_.next()) {}

GameScene::~GameScene() { cpu_.cancel(); }

void GameScene::decide() {
    hover_.reset();
    switch (game_.status()) {
    case TurnStatus::GameOver:
        phase_ = Phase::Finished;
        tween_ = Tween(anim::kFinishDelay);
        message_ = "対局終了！";
        break;
    case TurnStatus::EventDue:
        phase_ = Phase::EventLead;
        tween_ = Tween(anim::kEventLead);
        message_ = "盤面イベント発動！";
        break;
    case TurnStatus::NeedMove:
        if (game_.turn() == player_) {
            phase_ = Phase::WaitHuman;
            message_ = "あなたの番です。光っているマスに置けます。";
        } else {
            phase_ = Phase::WaitCpu;
            message_ = "CPU 思考中";
            cpu_.start(game_, difficulty_, seedRng_.next());
        }
        break;
    case TurnStatus::MustPass:
        phase_ = Phase::ShowPass;
        tween_ = Tween(anim::kPassMessage);
        message_ = game_.turn() == player_ ? "置ける場所がないので、あなたはパスです。" : "CPU はパスしました。";
        break;
    case TurnStatus::BothBlocked:
        phase_ = Phase::ShowBothPass;
        tween_ = Tween(anim::kPassMessage);
        message_ = "両者パス！ 予告のイベントがすぐに発動します。";
        break;
    }
}

void GameScene::startMoveAnim(const MoveResult& r, Color mover) {
    lastMove_ = r;
    lastMover_ = mover;
    phase_ = Phase::AnimFlip;
    tween_ = Tween(anim::kFlip);
}

void GameScene::startEvent(const EventResult& r) {
    lastEvent_ = r;
    phase_ = Phase::AnimEvent;
    tween_ = Tween(anim::kEvent);
    message_ = r.event.describe();
}

void GameScene::finish() {
    MatchOutcome o;
    o.finalBoard = game_.board();
    o.playerColor = player_;
    o.playerCount = game_.count(player_);
    o.cpuCount = game_.count(cpuColor());
    o.difficulty = difficulty_;
    app_.setOutcome(o);
    app_.changeScene(SceneId::Result);
}

void GameScene::update(double dt) {
    time_ += dt;
    const Input& in = app_.input();
    switch (phase_) {
    case Phase::Decide:
        decide();
        break;
    case Phase::WaitHuman: {
        hover_.reset();
        if (const auto p = view_.hit(in.x(), in.y()); p && game_.isLegal(*p)) hover_ = p;
        if (in.pressed() && hover_) {
            startMoveAnim(game_.play(*hover_), player_);
        } else if (debugOptions().autoplay) {  // Debug 検証用: プレイヤー側を自動で打つ
            autoplayTimer_ += dt;
            if (autoplayTimer_ >= 0.6) {
                autoplayTimer_ = 0.0;
                AiOptions opt;
                opt.difficulty = Difficulty::Weak;
                if (const auto m = chooseMove(game_, opt, seedRng_)) startMoveAnim(game_.play(*m), player_);
            }
        }
        break;
    }
    case Phase::WaitCpu:
        if (const auto r = cpu_.poll(dt)) {
            if (*r && game_.isLegal(**r)) {
                startMoveAnim(game_.play(**r), cpuColor());
            } else {
                phase_ = Phase::Decide;
            }
        }
        break;
    case Phase::AnimFlip:
    case Phase::AnimEvent:
        tween_.update(dt);
        if (tween_.done()) phase_ = Phase::Decide;
        break;
    case Phase::EventLead:
        tween_.update(dt);
        if (tween_.done()) startEvent(game_.fireDueEvent());
        break;
    case Phase::ShowPass:
        tween_.update(dt);
        if (tween_.done()) {
            game_.pass();
            phase_ = Phase::Decide;
        }
        break;
    case Phase::ShowBothPass:
        tween_.update(dt);
        if (tween_.done()) startEvent(game_.resolveBothBlocked());
        break;
    case Phase::Finished:
        tween_.update(dt);
        if (tween_.done()) finish();
        break;
    }
}

void GameScene::draw() {
    const Fonts& f = app_.fonts();
    DrawBox(0, 0, kScreenW, kScreenH, ui::colBackground(), TRUE);
    view_.drawBase(f.tiny);

    const bool showPreview = phase_ != Phase::AnimEvent && phase_ != Phase::Finished;
    const bool imminent = phase_ == Phase::EventLead || phase_ == Phase::ShowBothPass;
    if (showPreview) view_.drawEventBand(game_.nextEvent(), time_, imminent);

    switch (phase_) {
    case Phase::AnimFlip:
        view_.drawFlipAnim(game_.board(), lastMove_.placed, lastMove_.flipped, lastMover_, tween_.linear());
        break;
    case Phase::AnimEvent:
        view_.drawEventAnim(lastEvent_.before, lastEvent_.event, tween_.eased());
        break;
    default:
        view_.drawStones(game_.board());
        break;
    }

    if (phase_ != Phase::AnimEvent) {
        if (const auto lm = game_.lastMove()) view_.drawLastMove(*lm);
    }

    if (showPreview) view_.drawEventArrow(game_.nextEvent(), imminent);

    if (phase_ == Phase::WaitHuman) {
        view_.drawLegalMoves(game_.legalMoves(), time_);
        if (hover_) view_.drawHover(*hover_, player_, game_.flipsFor(*hover_));
    }

    if (phase_ == Phase::Finished) {
        const Rect r = view_.rect();
        ui::AlphaScope a(180);
        DrawBox(r.x, r.y + r.h / 2 - 50, r.x + r.w, r.y + r.h / 2 + 50, GetColor(0, 0, 0), TRUE);
    }
    if (phase_ == Phase::Finished) {
        const Rect r = view_.rect();
        ui::drawTextCentered(r.x + r.w / 2, r.y + r.h / 2 - 22, "対局終了", ui::colAccent(), f.large);
    }

    drawSidePanel();
}

void GameScene::drawSidePanel() {
    const Fonts& f = app_.fonts();
    constexpr int px = 760;
    constexpr int pw = 480;
    const BoardView icon(0, 0, 44);

    ui::drawText(px, 26, "SHIFT REVERSI", ui::colAccent(), f.medium);
    ui::drawText(px + 250, 32, std::format("CPU の強さ: {}", difficultyName(difficulty_)), ui::colSubText(), f.body);

    // プレイヤー情報と石数
    const Rect players{px, 70, pw, 160};
    ui::drawPanel(players, ui::colPanel(), ui::colPanelBorder());
    const bool waitingTurn = phase_ == Phase::WaitHuman || phase_ == Phase::WaitCpu;
    for (int i = 0; i < 2; ++i) {
        const Color c = i == 0 ? player_ : cpuColor();
        const int y = players.y + 14 + i * 74;
        if (waitingTurn && game_.turn() == c) {
            ui::AlphaScope a(60);
            DrawBox(players.x + 6, y - 4, players.x + players.w - 6, y + 62, ui::colAccent(), TRUE);
        }
        icon.drawStone(static_cast<float>(px + 40), static_cast<float>(y + 29), c);
        const std::string who = i == 0 ? "あなた" : "CPU";
        const std::string role = c == Cell::Black ? "（黒・先手）" : "（白・後手）";
        ui::drawText(px + 76, y + 6, who, ui::colText(), f.medium);
        ui::drawText(px + 76 + ui::textWidth(who, f.medium) + 6, y + 12, role, ui::colSubText(), f.body);
        const std::string count = std::to_string(game_.count(c));
        ui::drawText(players.x + players.w - 24 - ui::textWidth(count, f.large), y + 6, count, ui::colText(), f.large);
    }

    // 予告（FR-016）
    const Rect ev{px, 248, pw, 210};
    ui::drawPanel(ev, ui::colPanel(), ui::colPanelBorder());
    ui::drawText(px + 20, ev.y + 14, "次の盤面イベント（予告）", ui::colSubText(), f.body);
    // アニメーション中は Game 側で次の予告が抽選済みなので、発動中のイベントを表示する
    const BoardEvent& shown = phase_ == Phase::AnimEvent ? lastEvent_.event : game_.nextEvent();
    ui::drawText(px + 20, ev.y + 50, shown.describe(), ui::colText(), f.medium);
    const int remain = game_.movesUntilEvent();
    const bool firing = phase_ == Phase::EventLead || phase_ == Phase::AnimEvent || phase_ == Phase::ShowBothPass;
    if (firing) {
        ui::drawText(px + 20, ev.y + 100, "発動！", GetColor(255, 110, 80), f.large);
    } else {
        ui::drawText(px + 20, ev.y + 100, std::format("あと {} 手", remain), ui::colAccent(), f.large);
        if (remain == 1) {
            const int blink = static_cast<int>(std::fmod(time_ * 2.0, 2.0));
            if (blink == 0) ui::drawText(px + 20, ev.y + 158, "次の 1 手のあとに発動します！", GetColor(255, 120, 90), f.body);
        } else {
            ui::drawText(px + 20, ev.y + 158, "4 手ごとに発動（パスも 1 手）", ui::colSubText(), f.body);
        }
    }

    // 状況メッセージ
    const Rect msg{px, 476, pw, 110};
    ui::drawPanel(msg, ui::colPanel(), ui::colPanelBorder());
    std::string text = message_;
    if (phase_ == Phase::WaitCpu) {
        const int dots = static_cast<int>(time_ * 3.0) % 4;
        text += std::string(static_cast<size_t>(dots), '.');
    }
    ui::drawText(px + 20, msg.y + 20, text, ui::colText(), f.body);
    if (phase_ == Phase::WaitHuman) {
        ui::drawText(px + 20, msg.y + 60, "マウスを乗せると、ひっくり返る石が分かります。", ui::colSubText(), f.tiny);
    }

    ui::drawText(px, 606, "● 赤い点: 直前に置かれた石", ui::colSubText(), f.tiny);
    ui::drawText(px, 630, "■ 黄色の帯と矢印: 次に動く行・列／区画", ui::colSubText(), f.tiny);
}
