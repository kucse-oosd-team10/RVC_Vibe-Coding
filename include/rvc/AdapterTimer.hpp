#pragma once

#include "rvc/Interfaces.hpp"

#include <chrono>
#include <cstdint>
#include <functional>

namespace rvc {

class Timer final : public ITimer {
  public:
    using ClockFn = std::function<std::int64_t()>;
    using Callback = std::function<void()>;

    explicit Timer(ClockFn clockFn = defaultClock);

    void start(std::chrono::milliseconds duration) override;
    void start();
    void stop();
    void reset() override;
    [[nodiscard]] bool expired() const override;
    [[nodiscard]] bool isExpired() const;
    [[nodiscard]] bool isRunning() const;
    void setDuration(std::int64_t durationMs);
    void setCallback(Callback callback);
    void update();

  private:
    static std::int64_t defaultClock();

    ClockFn clock_;
    Callback callback_;
    std::chrono::milliseconds duration_{0};
    std::int64_t startTime_{0};
    bool running_{false};
    bool fired_{false};
};

} // namespace rvc
