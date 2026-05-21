#include "rvc/MovementManager.hpp"
#include "rvc/AdapterTimer.hpp"

namespace rvc {

MovementManager::MovementManager(IMovementMotor &motor, ITimer &turnTimer,
                                 std::chrono::milliseconds turnDuration)
    : motor_{&motor}, turnTimer_{&turnTimer}, turnDuration_{turnDuration} {}

MovementManager::MovementManager(IMotor &motor, IAvoidStrategy &strategy)
    : simulatorMotor_{&motor}, simulatorStrategy_{&strategy},
      ownedTurnTimer_{std::make_unique<Timer>()}, turnTimer_{ownedTurnTimer_.get()},
      turnDuration_{std::chrono::milliseconds{0}} {}

void MovementManager::stop() {
    currentCommand_ = MovementCommand::Stop;
    turnTimer_->reset();
    if (motor_ != nullptr) {
        motor_->stop();
        return;
    }
    simulatorMotor_->move(Direction::STOP);
}

void MovementManager::moveForward() {
    currentCommand_ = MovementCommand::Forward;
    turnTimer_->reset();
    if (motor_ != nullptr) {
        motor_->moveForward();
        return;
    }
    simulatorMotor_->move(Direction::FORWARD);
}

void MovementManager::moveBackward() {
    currentCommand_ = MovementCommand::Backward;
    turnTimer_->reset();
    if (motor_ != nullptr) {
        motor_->moveBackward();
        return;
    }
    simulatorMotor_->move(Direction::BACKWARD);
}

void MovementManager::turnLeft() {
    currentCommand_ = MovementCommand::TurnLeft;
    turnTimer_->start(turnDuration_);
    if (motor_ != nullptr) {
        motor_->turnLeft();
        return;
    }
    simulatorMotor_->move(Direction::LEFT);
}

void MovementManager::turnRight() {
    currentCommand_ = MovementCommand::TurnRight;
    turnTimer_->start(turnDuration_);
    if (motor_ != nullptr) {
        motor_->turnRight();
        return;
    }
    simulatorMotor_->move(Direction::RIGHT);
}

void MovementManager::turn(Direction direction) {
    if (direction == Direction::RIGHT) {
        turnRight();
        return;
    }
    turnLeft();
}

void MovementManager::executeAvoidance(bool front, bool left, bool right) {
    if (needsReverse(front, left, right)) {
        moveBackward();
        return;
    }
    turn(simulatorStrategy_->decideDirection(front, left, right));
}

bool MovementManager::needsReverse(bool front, bool left, bool right) const {
    return simulatorStrategy_ != nullptr && simulatorStrategy_->needsReverse(front, left, right);
}

bool MovementManager::isTurnComplete() const {
    if (currentCommand_ != MovementCommand::TurnLeft &&
        currentCommand_ != MovementCommand::TurnRight) {
        return false;
    }
    return turnTimer_->expired();
}

MovementCommand MovementManager::currentCommand() const { return currentCommand_; }

} // namespace rvc
