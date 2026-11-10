#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <random>

#include "Input.h"
#include "shift/Ai.h"

class Scene;

enum class SideChoice { Black, White, Random };
enum class SceneId { Title, Rules, Game, Result };

// タイトルで選ぶ対局設定（FR-023）。アプリ実行中は保持する。
struct MatchSettings {
    shift::Difficulty difficulty = shift::Difficulty::Normal;
    SideChoice side = SideChoice::Black;
};

// リザルト画面へ渡す結果（FR-026）
struct MatchOutcome {
    shift::Board finalBoard;
    shift::Color playerColor = shift::Cell::Black;
    int playerCount = 0;
    int cpuCount = 0;
    shift::Difficulty difficulty = shift::Difficulty::Normal;
};

struct Fonts {
    int title = -1;
    int headline = -1;
    int large = -1;
    int medium = -1;
    int body = -1;
    int tiny = -1;
};

const char* difficultyName(shift::Difficulty d);
const char* colorName(shift::Color c);

// シーン管理と共有リソース
class App {
public:
    App();
    ~App();
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    void update(double dt);
    void draw();

    void changeScene(SceneId id) { pending_ = id; }  // 次フレーム先頭で切り替え
    void requestQuit() { quit_ = true; }
    bool quitRequested() const { return quit_; }

    const Fonts& fonts() const { return fonts_; }
    const Input& input() const { return input_; }
    MatchSettings& settings() { return settings_; }
    void setOutcome(const MatchOutcome& o) { outcome_ = o; }
    const std::optional<MatchOutcome>& outcome() const { return outcome_; }
    std::uint32_t newSeed() { return static_cast<std::uint32_t>(seedEngine_()); }

private:
    void applyPendingScene();

    Fonts fonts_;
    Input input_;
    MatchSettings settings_;
    std::optional<MatchOutcome> outcome_;
    std::unique_ptr<Scene> scene_;
    std::optional<SceneId> pending_;
    std::mt19937 seedEngine_;
    bool quit_ = false;
};
