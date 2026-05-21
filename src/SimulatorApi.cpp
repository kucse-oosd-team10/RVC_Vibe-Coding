#include "rvc/SimulatorApi.hpp"

#include <string>

namespace rvc {

Direction DefaultAvoidStrategy::decideDirection(bool front, bool left, bool right) {
    const LeftPriorityAvoidanceStrategy strategy;
    const AvoidanceAction action =
        strategy.decideOnFrontObstacle({.leftDetected = left, .rightDetected = right});
    if (front && action == AvoidanceAction::TurnRight) {
        return Direction::RIGHT;
    }
    if (front && action == AvoidanceAction::MoveBackward) {
        return Direction::BACKWARD;
    }
    if (front) {
        return Direction::LEFT;
    }
    return Direction::FORWARD;
}

bool DefaultAvoidStrategy::needsReverse(bool front, bool left, bool right) {
    return front && left && right;
}

RVCController::RVCController(IObstacleSensor& obstacleSensor, IDustSensor& dustSensor,
                             IMotor& motor, ICleaner& cleaner, MovementManager& movementManager,
                             CleaningManager& cleaningManager,
                             ObstacleSensorSubject& obstacleSubject, DustSensorSubject& dustSubject)
    : obstacleSensor_{obstacleSensor}, dustSensor_{dustSensor}, motor_{motor}, cleaner_{cleaner},
      movementManager_{movementManager}, cleaningManager_{cleaningManager},
      obstacleSubject_{obstacleSubject}, dustSubject_{dustSubject}, frontAdapter_{obstacleSensor_},
      sideAdapter_{obstacleSensor_}, movementMotorAdapter_{motor_},
      cleaningMotorAdapter_{cleaner_}, core_{frontAdapter_,    sideAdapter_,     dustSensor_,
                                             movementManager_, cleaningManager_, defaultStrategy_} {
    obstacleSubject_.attach(this);
    dustSubject_.attach(this);
}

RVCController::~RVCController() {
    obstacleSubject_.detach(this);
    dustSubject_.detach(this);
}

void RVCController::powerOn() {
    error_ = false;
    if (!initializeDevices()) {
        error_ = true;
        motor_.move(Direction::STOP);
        cleaner_.setPower(PowerLevel::OFF);
        return;
    }
    core_.startCleaning();
}

void RVCController::powerOff() {
    error_ = false;
    core_.stopCleaning();
}

void RVCController::tick() {
    if (error_) {
        return;
    }
    obstacleSubject_.poll();
    dustSubject_.poll();
    core_.tick();
}

void RVCController::onObstacleDetected(bool front, bool, bool) {
    if (error_) {
        return;
    }
    if (front) {
        core_.onFrontObstacleDetected();
    }
}

void RVCController::onDustDetected(bool detected) {
    if (error_) {
        return;
    }
    if (detected) {
        core_.onDustDetected();
    }
}

MovementState RVCController::movementState() const {
    return core_.movementState();
}

bool RVCController::isError() const {
    return error_;
}

RvcController& RVCController::coreController() {
    return core_;
}

const RvcController& RVCController::coreController() const {
    return core_;
}

bool RVCController::initializeDevices() {
    for (int attempt = 0; attempt < kMaxInitializeAttempts; ++attempt) {
        const bool motorReady = motor_.initialize();
        const bool cleanerReady = cleaner_.initialize();
        const bool obstacleSensorReady = obstacleSensor_.initialize();
        const bool dustSensorReady = dustSensor_.initialize();
        if (motorReady && cleanerReady && obstacleSensorReady && dustSensorReady) {
            return true;
        }
    }
    return false;
}

std::string current_state_name(const RVCController& controller) {
    if (controller.isError()) {
        return "Error";
    }
    switch (controller.movementState()) {
        case MovementState::Off:
            return "Off";
        case MovementState::Forward:
            return "Cleaning";
        case MovementState::StoppedForObstacle:
        case MovementState::TurningLeft:
        case MovementState::TurningRight:
        case MovementState::Backward:
            return "Avoiding";
    }
    return "Unknown";
}

} // namespace rvc
