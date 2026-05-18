#pragma once

#include "rvc/Interfaces.hpp"
#include "rvc/Types.hpp"

#include <chrono>
#include <functional>
#include <vector>

namespace rvc::test {

class FakeTimer final : public ITimer {
  public:
    void start(std::chrono::milliseconds duration) override {
        duration_ = duration;
        running_ = true;
        expired_ = false;
        startCount_ += 1;
    }

    bool expired() const override { return running_ && expired_; }

    void reset() override {
        running_ = false;
        expired_ = false;
    }

    void expire() {
        if (running_) {
            expired_ = true;
        }
    }

    [[nodiscard]] int startCount() const { return startCount_; }

    [[nodiscard]] std::chrono::milliseconds duration() const { return duration_; }

  private:
    std::chrono::milliseconds duration_{0};
    bool running_{false};
    bool expired_{false};
    int startCount_{0};
};

class FakeFrontObstacleSensor final : public IFrontObstacleSensor {
  public:
    bool initialize() override {
        initialized = true;
        initializeCount += 1;
        return true;
    }

    void shutdown() override {
        initialized = false;
        shutdownCount += 1;
    }

    void registerInterruptHandler(InterruptHandler handler) override {
        interruptHandler = std::move(handler);
    }

    void triggerInterrupt() {
        if (interruptHandler) {
            interruptHandler();
        }
    }

    bool initialized{false};
    int initializeCount{0};
    int shutdownCount{0};
    InterruptHandler interruptHandler;
};

class FakeSideObstacleSensor final : public ISideObstacleSensor {
  public:
    bool initialize() override {
        initialized = true;
        initializeCount += 1;
        return true;
    }

    void shutdown() override {
        initialized = false;
        shutdownCount += 1;
    }

    SideObstacleSnapshot read() override {
        readCount += 1;
        if (queuedSnapshots.empty()) {
            return currentSnapshot;
        }
        const SideObstacleSnapshot snapshot = queuedSnapshots.front();
        queuedSnapshots.erase(queuedSnapshots.begin());
        currentSnapshot = snapshot;
        return snapshot;
    }

    bool initialized{false};
    int initializeCount{0};
    int shutdownCount{0};
    int readCount{0};
    SideObstacleSnapshot currentSnapshot{};
    std::vector<SideObstacleSnapshot> queuedSnapshots;
};

class FakeDustSensor final : public IDustSensor {
  public:
    bool initialize() override {
        initialized = true;
        initializeCount += 1;
        return true;
    }

    void shutdown() override {
        initialized = false;
        shutdownCount += 1;
    }

    bool isDustDetected() override {
        readCount += 1;
        return dustDetected;
    }

    bool initialized{false};
    bool dustDetected{false};
    int initializeCount{0};
    int shutdownCount{0};
    int readCount{0};
};

class FakeMovementMotor final : public IMovementMotor {
  public:
    void stop() override { commands.push_back(MovementCommand::Stop); }

    void moveForward() override { commands.push_back(MovementCommand::Forward); }

    void moveBackward() override { commands.push_back(MovementCommand::Backward); }

    void turnLeft() override { commands.push_back(MovementCommand::TurnLeft); }

    void turnRight() override { commands.push_back(MovementCommand::TurnRight); }

    [[nodiscard]] MovementCommand lastCommand() const {
        return commands.empty() ? MovementCommand::Stop : commands.back();
    }

    std::vector<MovementCommand> commands;
};

class FakeCleaningMotor final : public ICleaningMotor {
  public:
    void off() override { states.push_back(CleaningState::Off); }

    void normal() override { states.push_back(CleaningState::Normal); }

    void powerUp() override { states.push_back(CleaningState::PowerUp); }

    [[nodiscard]] CleaningState lastState() const {
        return states.empty() ? CleaningState::Off : states.back();
    }

    std::vector<CleaningState> states;
};

class FakeObstacleSensor final : public IObstacleSensor {
  public:
    bool initialize() override {
        initializeCount += 1;
        if (initializeFailuresRemaining > 0) {
            initializeFailuresRemaining -= 1;
            return false;
        }
        initialized = true;
        return true;
    }

    bool isFrontDetected() override {
        frontReadCount += 1;
        return frontDetected;
    }

    bool isLeftDetected() override {
        leftReadCount += 1;
        return leftDetected;
    }

    bool isRightDetected() override {
        rightReadCount += 1;
        return rightDetected;
    }

    bool initialized{false};
    bool frontDetected{false};
    bool leftDetected{false};
    bool rightDetected{false};
    int initializeFailuresRemaining{0};
    int initializeCount{0};
    int frontReadCount{0};
    int leftReadCount{0};
    int rightReadCount{0};
};

class FakeSimulatorMotor final : public IMotor {
  public:
    bool initialize() override {
        initializeCount += 1;
        if (initializeFailuresRemaining > 0) {
            initializeFailuresRemaining -= 1;
            return false;
        }
        initialized = true;
        return true;
    }

    void move(Direction direction) override { commands.push_back(direction); }

    [[nodiscard]] Direction lastCommand() const {
        return commands.empty() ? Direction::STOP : commands.back();
    }

    bool initialized{false};
    int initializeFailuresRemaining{0};
    int initializeCount{0};
    std::vector<Direction> commands;
};

class FakeCleaner final : public ICleaner {
  public:
    bool initialize() override {
        initializeCount += 1;
        if (initializeFailuresRemaining > 0) {
            initializeFailuresRemaining -= 1;
            return false;
        }
        initialized = true;
        return true;
    }

    void setPower(PowerLevel level) override { levels.push_back(level); }

    [[nodiscard]] PowerLevel lastPower() const {
        return levels.empty() ? PowerLevel::OFF : levels.back();
    }

    bool initialized{false};
    int initializeFailuresRemaining{0};
    int initializeCount{0};
    std::vector<PowerLevel> levels;
};

} // namespace rvc::test
