#include "rvc/AdapterTimer.hpp"

#include <chrono>
#include <utility>

namespace rvc {

Timer::Timer(ClockFn clockFn) : clock_{std::move(clockFn)} {}

void Timer::start(std::chrono::milliseconds duration) {
    duration_ = duration;
    start();
}

void Timer::start() {
    startTime_ = clock_();
    running_ = true;
    fired_ = false;
}

void Timer::stop() { running_ = false; }

void Timer::reset() {
    running_ = false;
    fired_ = false;
}

bool Timer::expired() const { return isExpired(); }

bool Timer::isExpired() const {
    if (!running_) {
        return false;
    }
    return clock_() - startTime_ >= duration_.count();
}

bool Timer::isRunning() const { return running_; }

void Timer::setDuration(std::int64_t durationMs) {
    duration_ = std::chrono::milliseconds{durationMs};
}

void Timer::setCallback(Callback callback) { callback_ = std::move(callback); }

void Timer::update() {
    if (!isExpired() || fired_) {
        return;
    }
    fired_ = true;
    running_ = false;
    if (callback_) {
        callback_();
    }
}

std::int64_t Timer::defaultClock() {
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

} // namespace rvc
