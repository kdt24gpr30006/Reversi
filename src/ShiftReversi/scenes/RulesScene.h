#pragma once
#include "scenes/Scene.h"

// ルール説明画面（US3, FR-027）
class RulesScene : public Scene {
public:
    explicit RulesScene(App& app);
    void update(double dt) override;
    void draw() override;

    static constexpr int kPageCount = 5;

private:
    void drawDiagram() const;

    int page_ = 0;
    double time_ = 0.0;
};
