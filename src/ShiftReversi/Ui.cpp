#include "Ui.h"

#include <cstring>

#include "DxLib.h"
#include "Input.h"

namespace ui {

unsigned int colBackground() { return GetColor(24, 32, 40); }
unsigned int colText() { return GetColor(240, 240, 235); }
unsigned int colSubText() { return GetColor(170, 180, 190); }
unsigned int colAccent() { return GetColor(255, 196, 64); }
unsigned int colPanel() { return GetColor(36, 48, 60); }
unsigned int colPanelBorder() { return GetColor(70, 90, 110); }

int textWidth(const std::string& s, int font) {
    return GetDrawStringWidthToHandle(s.c_str(), static_cast<int>(s.size()), font);
}

void drawText(int x, int y, const std::string& s, unsigned int color, int font) {
    DrawStringToHandle(x, y, s.c_str(), color, font);
}

void drawTextCentered(int cx, int y, const std::string& s, unsigned int color, int font) {
    DrawStringToHandle(cx - textWidth(s, font) / 2, y, s.c_str(), color, font);
}

void drawPanel(const Rect& r, unsigned int fill, unsigned int border) {
    DrawRoundRect(r.x, r.y, r.x + r.w, r.y + r.h, 10, 10, fill, TRUE);
    DrawRoundRect(r.x, r.y, r.x + r.w, r.y + r.h, 10, 10, border, FALSE);
}

void drawButton(const Rect& r, const std::string& label, int font, const Input& input, ButtonState state) {
    const bool hovered = state != ButtonState::Disabled && r.contains(input.x(), input.y());
    unsigned int fill = GetColor(52, 70, 88);
    unsigned int border = GetColor(90, 115, 140);
    unsigned int text = colText();
    if (state == ButtonState::Selected) {
        fill = GetColor(200, 150, 40);
        border = colAccent();
        text = GetColor(20, 20, 20);
    } else if (state == ButtonState::Disabled) {
        fill = GetColor(40, 46, 52);
        border = GetColor(60, 66, 72);
        text = GetColor(100, 106, 112);
    } else if (hovered) {
        fill = GetColor(72, 98, 124);
        border = colAccent();
    }
    drawPanel(r, fill, border);
    const int fontH = GetFontSizeToHandle(font);
    drawTextCentered(r.x + r.w / 2, r.y + (r.h - fontH) / 2, label, text, font);
}

bool clicked(const Rect& r, const Input& input, ButtonState state) {
    return state != ButtonState::Disabled && input.pressed() && r.contains(input.x(), input.y());
}

AlphaScope::AlphaScope(int alpha) { SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha); }
AlphaScope::~AlphaScope() { SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); }

}  // namespace ui
