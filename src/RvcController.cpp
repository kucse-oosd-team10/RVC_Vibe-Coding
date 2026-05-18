#include "rvc/RvcController.hpp"

namespace rvc {

void IRvcState::onEnter(RvcController &) {}

void IRvcState::startCleaning(RvcController &) {}

void IRvcState::stopCleaning(RvcController &context) { context.performStop(); }

void IRvcState::onFrontObstacleDetected(RvcController &) {}

void IRvcState::onDustDetected(RvcController &context) {
    context.cleaningManager().onDustDetected(movementState());
}

void IRvcState::tick(RvcController &) {}

RvcController::RvcController(IFrontObstacleSensor &frontSensor, ISideObstacleSensor &sideSensor,
                             IDustSensor &dustSensor, MovementManager &movementManager,
                             CleaningManager &cleaningManager,
                             IObstacleAvoidanceStrategy &avoidanceStrategy)
    : frontSensor_{frontSensor}, sideSensor_{sideSensor}, dustSensor_{dustSensor},
      movementManager_{movementManager}, cleaningManager_{cleaningManager},
      avoidanceStrategy_{avoidanceStrategy}, currentState_{makeOffState()} {}

void RvcController::startCleaning() {
    enteringState_ = true;
    currentState_->startCleaning(*this);
    enteringState_ = false;
    enterPendingStateIfNeeded();
}

void RvcController::stopCleaning() { performStop(); }

void RvcController::onFrontObstacleDetected() {
    enteringState_ = true;
    currentState_->onFrontObstacleDetected(*this);
    enteringState_ = false;
    enterPendingStateIfNeeded();
}

void RvcController::onDustDetected() {
    enteringState_ = true;
    currentState_->onDustDetected(*this);
    enteringState_ = false;
    enterPendingStateIfNeeded();
}

void RvcController::tick() {
    enteringState_ = true;
    currentState_->tick(*this);
    enteringState_ = false;
    enterPendingStateIfNeeded();
    cleaningManager_.tick();
}

void RvcController::changeState(std::unique_ptr<IRvcState> nextState) {
    if (enteringState_) {
        pendingState_ = std::move(nextState);
        return;
    }

    enterState(std::move(nextState));
}

void RvcController::enterState(std::unique_ptr<IRvcState> nextState) {
    currentState_ = std::move(nextState);
    do {
        enteringState_ = true;
        currentState_->onEnter(*this);
        enteringState_ = false;

        if (!pendingState_) {
            break;
        }

        currentState_ = std::move(pendingState_);
    } while (true);
}

void RvcController::enterPendingStateIfNeeded() {
    if (pendingState_) {
        enterState(std::move(pendingState_));
    }
}

void RvcController::performStop() {
    movementManager_.stop();
    cleaningManager_.stop();
    frontSensor_.shutdown();
    sideSensor_.shutdown();
    dustSensor_.shutdown();
    currentState_ = makeOffState();
}

MovementState RvcController::movementState() const { return currentState_->movementState(); }

IFrontObstacleSensor &RvcController::frontSensor() { return frontSensor_; }

ISideObstacleSensor &RvcController::sideSensor() { return sideSensor_; }

IDustSensor &RvcController::dustSensor() { return dustSensor_; }

MovementManager &RvcController::movementManager() { return movementManager_; }

CleaningManager &RvcController::cleaningManager() { return cleaningManager_; }

IObstacleAvoidanceStrategy &RvcController::avoidanceStrategy() { return avoidanceStrategy_; }

} // namespace rvc
