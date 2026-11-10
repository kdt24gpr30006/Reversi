#pragma once
// 検証用の起動オプション（Debug ビルドでのみ解析される。Release では常に既定値で、製品の動作は変わらない）
//   --scene=title|rules|game   開始画面
//   --rules-page=N             ルール説明の開始ページ（0 始まり）
//   --side=black|white         --scene=game 時のプレイヤーの色
//   --autoplay                 プレイヤー側も CPU（弱）が自動で打つ
//   --capture=DIR              一定間隔で画面を PNG 保存
//   --capture-interval=SEC     保存間隔（既定 1.0）
//   --quit-after=SEC           指定秒後に終了
#include <sstream>
#include <string>

struct DebugOptions {
    std::string startScene;
    int rulesPage = 0;
    std::string side;
    bool autoplay = false;
    std::string captureDir;
    double captureInterval = 1.0;
    double quitAfter = 0.0;
};

inline DebugOptions& debugOptions() {
    static DebugOptions o;
    return o;
}

inline void parseDebugOptions(const char* cmdLine) {
    DebugOptions& o = debugOptions();
    std::istringstream ss(cmdLine ? cmdLine : "");
    std::string tok;
    const auto value = [&tok](const char* key) -> std::string {
        const std::string k = std::string(key) + "=";
        return tok.rfind(k, 0) == 0 ? tok.substr(k.size()) : std::string();
    };
    while (ss >> tok) {
        if (tok == "--autoplay") o.autoplay = true;
        else if (auto v = value("--scene"); !v.empty()) o.startScene = v;
        else if (auto v2 = value("--rules-page"); !v2.empty()) o.rulesPage = std::stoi(v2);
        else if (auto v3 = value("--side"); !v3.empty()) o.side = v3;
        else if (auto v4 = value("--capture"); !v4.empty()) o.captureDir = v4;
        else if (auto v5 = value("--capture-interval"); !v5.empty()) o.captureInterval = std::stod(v5);
        else if (auto v6 = value("--quit-after"); !v6.empty()) o.quitAfter = std::stod(v6);
    }
}
