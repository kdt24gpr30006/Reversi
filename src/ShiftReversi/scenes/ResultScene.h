#pragma once
#include "scenes/Scene.h"

// リザルト画面（US2, FR-026）
class ResultScene : public Scene {
public:
    explicit ResultScene(App& app) : Scene(app) {}
    void update(double dt) override;
    void draw() override;
};
