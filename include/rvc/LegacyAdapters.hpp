#pragma once

#include "rvc/Interfaces.hpp"

namespace rvc {

class CombinedFrontObstacleSensorAdapter final : public IFrontObstacleSensor {
  public:
    explicit CombinedFrontObstacleSensorAdapter(IObstacleSensor &sensor);

    bool initialize() override;
    void shutdown() override;
    void registerInterruptHandler(InterruptHandler handler) override;
    void triggerInterrupt();

  private:
    IObstacleSensor &sensor_;
    InterruptHandler handler_;
};

class CombinedSideObstacleSensorAdapter final : public ISideObstacleSensor {
  public:
    explicit CombinedSideObstacleSensorAdapter(IObstacleSensor &sensor);

    bool initialize() override;
    void shutdown() override;
    SideObstacleSnapshot read() override;

  private:
    IObstacleSensor &sensor_;
};

class MovementMotorAdapter final : public IMovementMotor {
  public:
    explicit MovementMotorAdapter(IMotor &motor);

    void stop() override;
    void moveForward() override;
    void moveBackward() override;
    void turnLeft() override;
    void turnRight() override;

  private:
    IMotor &motor_;
};

class CleaningMotorAdapter final : public ICleaningMotor {
  public:
    explicit CleaningMotorAdapter(ICleaner &cleaner);

    void off() override;
    void normal() override;
    void powerUp() override;

  private:
    ICleaner &cleaner_;
};

} // namespace rvc
