#include "CpuWorker.h"

#include <chrono>

#include "Animation.h"

CpuWorker::~CpuWorker() { cancel(); }

void CpuWorker::start(const shift::Game& game, shift::Difficulty difficulty, std::uint32_t seed) {
    cancel();
    cancel_ = false;
    elapsed_ = 0.0;
    running_ = true;
    future_ = std::async(std::launch::async, [game, difficulty, seed, this]() {
        shift::AiOptions opt;
        opt.difficulty = difficulty;
        opt.cancel = &cancel_;
        shift::Random rng(seed);
        return shift::chooseMove(game, opt, rng);
    });
}

std::optional<std::optional<shift::Pos>> CpuWorker::poll(double dt) {
    if (!running_) return std::nullopt;
    elapsed_ += dt;
    if (elapsed_ < anim::kCpuMinDelay) return std::nullopt;
    if (future_.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return std::nullopt;
    running_ = false;
    return future_.get();
}

void CpuWorker::cancel() {
    cancel_ = true;
    if (future_.valid()) future_.wait();
    future_ = {};
    running_ = false;
}
