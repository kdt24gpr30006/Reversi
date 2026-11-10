#pragma once
#include <optional>
#include <string>

#include "Animation.h"
#include "BoardView.h"
#include "CpuWorker.h"
#include "scenes/Scene.h"
#include "shift/Game.h"

// 対局画面（US1）
class GameScene : public Scene {
public:
    explicit GameScene(App& app);
    ~GameScene() override;

    void update(double dt) override;
    void draw() override;

private:
    enum class Phase { Decide, WaitHuman, WaitCpu, AnimFlip, EventLead, AnimEvent, ShowPass, ShowBothPass, Finished };

    void decide();
    void startMoveAnim(const shift::MoveResult& r, shift::Color mover);
    void startEvent(const shift::EventResult& r);
    void finish();
    void drawSidePanel();

    shift::Color cpuColor() const { return shift::opponent(player_); }

    shift::Random seedRng_;
    shift::Color player_;
    shift::Difficulty difficulty_;
    shift::Game game_;
    BoardView view_{60, 44, 64};
    CpuWorker cpu_;

    Phase phase_ = Phase::Decide;
    Tween tween_;
    double time_ = 0.0;
    double autoplayTimer_ = 0.0;
    std::string message_;
    std::optional<shift::Pos> hover_;
    shift::MoveResult lastMove_;
    shift::Color lastMover_ = shift::Cell::Black;
    shift::EventResult lastEvent_;
};
