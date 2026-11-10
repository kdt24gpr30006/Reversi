#pragma once
#include <atomic>
#include <future>
#include <optional>

#include "shift/Ai.h"

// CPU 思考をワーカースレッドで行う（FR-020, 憲章 V）
class CpuWorker {
public:
    CpuWorker() = default;
    ~CpuWorker();
    CpuWorker(const CpuWorker&) = delete;
    CpuWorker& operator=(const CpuWorker&) = delete;

    void start(const shift::Game& game, shift::Difficulty difficulty, std::uint32_t seed);
    bool running() const { return running_; }

    // 思考完了かつ最低待ち時間経過で結果を返す（毎フレーム呼ぶ）
    std::optional<std::optional<shift::Pos>> poll(double dt);

    void cancel();

private:
    std::future<std::optional<shift::Pos>> future_;
    std::atomic<bool> cancel_{false};
    double elapsed_ = 0.0;
    bool running_ = false;
};
