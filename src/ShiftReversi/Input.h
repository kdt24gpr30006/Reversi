#pragma once

// マウス入力（位置と左ボタンの押下エッジ）
class Input {
public:
    void update();
    int x() const { return x_; }
    int y() const { return y_; }
    bool pressed() const { return down_ && !prevDown_; }  // このフレームで押された

private:
    int x_ = 0;
    int y_ = 0;
    bool down_ = false;
    bool prevDown_ = false;
};
