#pragma once
#include "scenes/Scene.h"

// タイトル画面（US2, FR-023）
class TitleScene : public Scene {
public:
    explicit TitleScene(App& app) : Scene(app) {}
    void update(double dt) override;
    void draw() override;

private:
    double time_ = 0.0;
};
