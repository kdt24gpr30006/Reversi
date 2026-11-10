#pragma once

class App;

// 画面（シーン）の基底クラス
class Scene {
public:
    explicit Scene(App& app) : app_(app) {}
    virtual ~Scene() = default;
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    virtual void update(double dt) = 0;
    virtual void draw() = 0;

protected:
    App& app_;
};
