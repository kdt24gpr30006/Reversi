#include "Input.h"

#include "DxLib.h"

void Input::update() {
    prevDown_ = down_;
    // 非アクティブ時（他ウィンドウ操作中）の入力は無視する
    if (GetWindowActiveFlag() == FALSE) {
        x_ = y_ = -1;
        down_ = false;
        return;
    }
    GetMousePoint(&x_, &y_);
    down_ = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
}
