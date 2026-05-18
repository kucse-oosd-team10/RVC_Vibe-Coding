#pragma once

#include "rvc/AdapterTimer.hpp"
#include "rvc/Interfaces.hpp"
#include "rvc/Types.hpp"

#include <chrono>
#include <memory>

namespace rvc {

class CleaningManager {
  public:
    CleaningManager(ICleaningMotor &cleaningMotor, IDustSensor &dustSensor, ITimer &powerUpTimer,
                    std::chrono::milliseconds powerUpDuration = kDefaultPowerUpDuration);
    CleaningManager(ICleaner &cleaner, Timer::ClockFn clockFn);

    void start();
    void startCleaning();
    void stop();
    void stopCleaning();
    void powerUp();
    void handleDustDetected(bool detected);
    void update();
    void onDustDetected(MovementState movementState);
    void onMovementStateChanged(MovementState movementState);
    void tick();

    [[nodiscard]] CleaningState currentState() const;
    [[nodiscard]] PowerLevel getPowerLevel() const;
    [[nodiscard]] bool getLatestDustDetected() const;
    [[nodiscard]] bool pendingPowerUp() const;

  private:
    void enterNormal();
    void enterPowerUp();

    ICleaningMotor *cleaningMotor_{nullptr};
    ICleaner *cleaner_{nullptr};
    IDustSensor *dustSensor_{nullptr};
    std::unique_ptr<Timer> ownedPowerUpTimer_;
    ITimer *powerUpTimer_{nullptr};
    CleaningState currentState_{CleaningState::Off};
    PowerLevel powerLevel_{PowerLevel::OFF};
    bool latestDustDetected_{false};
    bool pendingPowerUp_{false};
    std::chrono::milliseconds powerUpDuration_;
};

} // namespace rvc
