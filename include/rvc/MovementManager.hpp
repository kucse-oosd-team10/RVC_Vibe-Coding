#pragma once

#include "rvc/IAvoidStrategy.hpp"
#include "rvc/Interfaces.hpp"
#include "rvc/Types.hpp"

#include <chrono>
#include <memory>

namespace rvc {

class MovementManager {
  public:
    MovementManager(IMovementMotor &motor, ITimer &turnTimer,
                    std::chrono::milliseconds turnDuration = kDefaultTurnDuration);
    MovementManager(IMotor &motor, IAvoidStrategy &strategy);

    void stop();
    void moveForward();
    void moveBackward();
    void turnLeft();
    void turnRight();
    void turn(Direction direction);
    void executeAvoidance(bool front, bool left, bool right);
    [[nodiscard]] bool needsReverse(bool front, bool left, bool right) const;
    [[nodiscard]] bool isTurnComplete() const;

    [[nodiscard]] MovementCommand currentCommand() const;

  private:
    IMovementMotor *motor_{nullptr};
    IMotor *simulatorMotor_{nullptr};
    IAvoidStrategy *simulatorStrategy_{nullptr};
    std::unique_ptr<ITimer> ownedTurnTimer_;
    ITimer *turnTimer_{nullptr};
    MovementCommand currentCommand_{MovementCommand::Stop};
    std::chrono::milliseconds turnDuration_;
};

} // namespace rvc
