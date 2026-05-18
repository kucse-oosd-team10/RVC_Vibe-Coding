#include "rvc/LegacyAdapters.hpp"

#include <utility>

namespace rvc {

CombinedFrontObstacleSensorAdapter::CombinedFrontObstacleSensorAdapter(IObstacleSensor& sensor)
    : sensor_{sensor} {
}

bool CombinedFrontObstacleSensorAdapter::initialize() {
    return sensor_.initialize();
}

void CombinedFrontObstacleSensorAdapter::shutdown() {
}

void CombinedFrontObstacleSensorAdapter::registerInterruptHandler(InterruptHandler handler) {
    handler_ = std::move(handler);
}

void CombinedFrontObstacleSensorAdapter::triggerInterrupt() {
    if (handler_) {
        handler_();
    }
}

CombinedSideObstacleSensorAdapter::CombinedSideObstacleSensorAdapter(IObstacleSensor& sensor)
    : sensor_{sensor} {
}

bool CombinedSideObstacleSensorAdapter::initialize() {
    return sensor_.initialize();
}

void CombinedSideObstacleSensorAdapter::shutdown() {
}

SideObstacleSnapshot CombinedSideObstacleSensorAdapter::read() {
    return {.leftDetected = sensor_.isLeftDetected(), .rightDetected = sensor_.isRightDetected()};
}

MovementMotorAdapter::MovementMotorAdapter(IMotor& motor) : motor_{motor} {
}

void MovementMotorAdapter::stop() {
    motor_.move(Direction::STOP);
}

void MovementMotorAdapter::moveForward() {
    motor_.move(Direction::FORWARD);
}

void MovementMotorAdapter::moveBackward() {
    motor_.move(Direction::BACKWARD);
}

void MovementMotorAdapter::turnLeft() {
    motor_.move(Direction::LEFT);
}

void MovementMotorAdapter::turnRight() {
    motor_.move(Direction::RIGHT);
}

CleaningMotorAdapter::CleaningMotorAdapter(ICleaner& cleaner) : cleaner_{cleaner} {
}

void CleaningMotorAdapter::off() {
    cleaner_.setPower(PowerLevel::OFF);
}

void CleaningMotorAdapter::normal() {
    cleaner_.setPower(PowerLevel::NORMAL);
}

void CleaningMotorAdapter::powerUp() {
    cleaner_.setPower(PowerLevel::POWER_UP);
}

} // namespace rvc
