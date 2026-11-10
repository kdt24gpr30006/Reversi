#pragma once
#include <algorithm>

// 一定時間で 0→1 に進む補間（FR-028）
class Tween {
public:
    Tween() = default;
    explicit Tween(double duration) : duration_(duration) {}

    void update(double dt) { elapsed_ = std::min(elapsed_ + dt, duration_); }
    bool done() const { return elapsed_ >= duration_; }
    double linear() const { return duration_ <= 0.0 ? 1.0 : elapsed_ / duration_; }
    // ゆっくり始まりゆっくり終わる
    double eased() const {
        const double t = linear();
        return t * t * (3.0 - 2.0 * t);
    }

private:
    double duration_ = 0.0;
    double elapsed_ = 0.0;
};

namespace anim {
constexpr double kFlip = 0.3;       // 石の反転
constexpr double kEvent = 0.6;      // スライド・区画回転
constexpr double kEventLead = 0.35; // 発動前の「発動！」強調
constexpr double kPassMessage = 1.5;
constexpr double kCpuMinDelay = 0.5;  // FR-021
constexpr double kFinishDelay = 1.2;
}  // namespace anim
