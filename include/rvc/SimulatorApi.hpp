#pragma once

#include "rvc/AdapterTimer.hpp"
#include "rvc/CleaningManager.hpp"
#include "rvc/IAvoidStrategy.hpp"
#include "rvc/IObstacleAvoidanceStrategy.hpp"
#include "rvc/Interfaces.hpp"
#include "rvc/LegacyAdapters.hpp"
#include "rvc/MovementManager.hpp"
#include "rvc/RvcController.hpp"
#include "rvc/SensorSubjects.hpp"

#include <memory>
#include <string>

namespace rvc {

class RVCController final : public ISensorObserver {
  public:
    RVCController(IObstacleSensor &obstacleSensor, IDustSensor &dustSensor, IMotor &motor,
                  ICleaner &cleaner, MovementManager &movementManager,
                  CleaningManager &cleaningManager, ObstacleSensorSubject &obstacleSubject,
                  DustSensorSubject &dustSubject);
    ~RVCController() override;

    void powerOn();
    void powerOff();
    void tick();

    void onObstacleDetected(bool front, bool left, bool right) override;
    void onDustDetected(bool detected) override;

    [[nodiscard]] MovementState movementState() const;
    [[nodiscard]] bool isError() const;
    [[nodiscard]] RvcController &coreController();
    [[nodiscard]] const RvcController &coreController() const;

  private:
    [[nodiscard]] bool initializeDevices();

    static constexpr int kMaxInitializeAttempts{3};

    IObstacleSensor &obstacleSensor_;
    IDustSensor &dustSensor_;
    IMotor &motor_;
    ICleaner &cleaner_;
    MovementManager &movementManager_;
    CleaningManager &cleaningManager_;
    ObstacleSensorSubject &obstacleSubject_;
    DustSensorSubject &dustSubject_;

    CombinedFrontObstacleSensorAdapter frontAdapter_;
    CombinedSideObstacleSensorAdapter sideAdapter_;
    MovementMotorAdapter movementMotorAdapter_;
    CleaningMotorAdapter cleaningMotorAdapter_;
    LeftPriorityAvoidanceStrategy defaultStrategy_;
    RvcController core_;
    bool error_{false};
};

std::string current_state_name(RVCController &controller);

} // namespace rvc
