#include "App.h"

#include "DebugOptions.h"
#include "DxLib.h"
#include "scenes/GameScene.h"
#include "scenes/ResultScene.h"
#include "scenes/RulesScene.h"
#include "scenes/TitleScene.h"

const char* difficultyName(shift::Difficulty d) {
    switch (d) {
    case shift::Difficulty::Weak: return "弱";
    case shift::Difficulty::Normal: return "中";
    case shift::Difficulty::Strong: return "強";
    }
    return "";
}

const char* colorName(shift::Color c) { return c == shift::Cell::Black ? "黒" : "白"; }

App::App() : seedEngine_(std::random_device{}()) {
    const char* face = "メイリオ";
    fonts_.title = CreateFontToHandle(face, 84, 9, DX_FONTTYPE_ANTIALIASING_EDGE_8X8, -1, 3);
    fonts_.headline = CreateFontToHandle(face, 58, 8, DX_FONTTYPE_ANTIALIASING_EDGE_8X8, -1, 2);
    fonts_.large = CreateFontToHandle(face, 40, 6, DX_FONTTYPE_ANTIALIASING_8X8);
    fonts_.medium = CreateFontToHandle(face, 26, 5, DX_FONTTYPE_ANTIALIASING_8X8);
    fonts_.body = CreateFontToHandle(face, 21, 4, DX_FONTTYPE_ANTIALIASING_8X8);
    fonts_.tiny = CreateFontToHandle(face, 16, 3, DX_FONTTYPE_ANTIALIASING_8X8);
    const std::string& start = debugOptions().startScene;  // Debug 検証用（Release では常に空）
    pending_ = start == "rules" ? SceneId::Rules : start == "game" ? SceneId::Game : SceneId::Title;
    if (debugOptions().side == "white") settings_.side = SideChoice::White;
    applyPendingScene();
}

App::~App() {
    scene_.reset();
    InitFontToHandle();
}

void App::applyPendingScene() {
    if (!pending_) return;
    const SceneId id = *pending_;
    pending_.reset();
    scene_.reset();  // 先に破棄（CPU スレッドの停止を含む）
    switch (id) {
    case SceneId::Title: scene_ = std::make_unique<TitleScene>(*this); break;
    case SceneId::Rules: scene_ = std::make_unique<RulesScene>(*this); break;
    case SceneId::Game: scene_ = std::make_unique<GameScene>(*this); break;
    case SceneId::Result: scene_ = std::make_unique<ResultScene>(*this); break;
    }
}

void App::update(double dt) {
    applyPendingScene();
    input_.update();
    if (scene_) scene_->update(dt);
}

void App::draw() {
    if (scene_) scene_->draw();
}
