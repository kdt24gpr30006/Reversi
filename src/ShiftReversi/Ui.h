#pragma once
#include <string>

class Input;

constexpr int kScreenW = 1280;
constexpr int kScreenH = 720;

struct Rect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    bool contains(int px, int py) const { return px >= x && px < x + w && py >= y && py < y + h; }
};

namespace ui {

enum class ButtonState { Normal, Selected, Disabled };

// 配色
unsigned int colBackground();
unsigned int colText();
unsigned int colSubText();
unsigned int colAccent();
unsigned int colPanel();
unsigned int colPanelBorder();

int textWidth(const std::string& s, int font);
void drawText(int x, int y, const std::string& s, unsigned int color, int font);
void drawTextCentered(int cx, int y, const std::string& s, unsigned int color, int font);
void drawPanel(const Rect& r, unsigned int fill, unsigned int border);

// ボタン描画。ホバーは input の位置から判定する。
void drawButton(const Rect& r, const std::string& label, int font, const Input& input,
                ButtonState state = ButtonState::Normal);
// 有効なボタンがこのフレームでクリックされたか
bool clicked(const Rect& r, const Input& input, ButtonState state = ButtonState::Normal);

// 半透明描画のスコープ
class AlphaScope {
public:
    explicit AlphaScope(int alpha);
    ~AlphaScope();
    AlphaScope(const AlphaScope&) = delete;
    AlphaScope& operator=(const AlphaScope&) = delete;
};

}  // namespace ui
