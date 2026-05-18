#include "rvc/CleaningManager.hpp"

#include <utility>

namespace rvc {

CleaningManager::CleaningManager(ICleaningMotor &cleaningMotor, IDustSensor &dustSensor,
                                 ITimer &powerUpTimer, std::chrono::milliseconds powerUpDuration)
    : cleaningMotor_{&cleaningMotor}, dustSensor_{&dustSensor}, powerUpTimer_{&powerUpTimer},
      powerUpDuration_{powerUpDuration} {}

CleaningManager::CleaningManager(ICleaner &cleaner, Timer::ClockFn clockFn)
    : cleaner_{&cleaner}, ownedPowerUpTimer_{std::make_unique<Timer>(std::move(clockFn))},
      powerUpTimer_{ownedPowerUpTimer_.get()}, powerUpDuration_{kDefaultPowerUpDuration} {}

void CleaningManager::start() {
    pendingPowerUp_ = false;
    enterNormal();
}

void CleaningManager::startCleaning() { start(); }

void CleaningManager::stop() {
    pendingPowerUp_ = false;
    powerUpTimer_->reset();
    currentState_ = CleaningState::Off;
    powerLevel_ = PowerLevel::OFF;
    if (cleaningMotor_ != nullptr) {
        cleaningMotor_->off();
        return;
    }
    cleaner_->setPower(PowerLevel::OFF);
}

void CleaningManager::stopCleaning() { stop(); }

void CleaningManager::powerUp() { enterPowerUp(); }

void CleaningManager::handleDustDetected(bool detected) {
    latestDustDetected_ = detected;
    if (detected) {
        enterPowerUp();
    }
}

void CleaningManager::update() { tick(); }

void CleaningManager::onDustDetected(MovementState movementState) {
    if (currentState_ == CleaningState::Off) {
        return;
    }

    if (movementState == MovementState::Forward) {
        pendingPowerUp_ = false;
        enterPowerUp();
        return;
    }

    pendingPowerUp_ = true;
    enterNormal();
}

void CleaningManager::onMovementStateChanged(MovementState movementState) {
    if (currentState_ == CleaningState::Off) {
        return;
    }

    if (movementState != MovementState::Forward) {
        enterNormal();
        return;
    }

    if (pendingPowerUp_) {
        pendingPowerUp_ = false;
        enterPowerUp();
    }
}

void CleaningManager::tick() {
    if (currentState_ != CleaningState::PowerUp || !powerUpTimer_->expired()) {
        return;
    }

    const bool dustDetected =
        dustSensor_ != nullptr ? dustSensor_->isDustDetected() : latestDustDetected_;
    if (dustDetected) {
        powerUpTimer_->start(powerUpDuration_);
        return;
    }

    enterNormal();
}

CleaningState CleaningManager::currentState() const { return currentState_; }

PowerLevel CleaningManager::getPowerLevel() const { return powerLevel_; }

bool CleaningManager::getLatestDustDetected() const { return latestDustDetected_; }

bool CleaningManager::pendingPowerUp() const { return pendingPowerUp_; }

void CleaningManager::enterNormal() {
    powerUpTimer_->reset();
    currentState_ = CleaningState::Normal;
    powerLevel_ = PowerLevel::NORMAL;
    if (cleaningMotor_ != nullptr) {
        cleaningMotor_->normal();
        return;
    }
    cleaner_->setPower(PowerLevel::NORMAL);
}

void CleaningManager::enterPowerUp() {
    currentState_ = CleaningState::PowerUp;
    powerLevel_ = PowerLevel::POWER_UP;
    powerUpTimer_->start(powerUpDuration_);
    if (cleaningMotor_ != nullptr) {
        cleaningMotor_->powerUp();
        return;
    }
    cleaner_->setPower(PowerLevel::POWER_UP);
}

} // namespace rvc
