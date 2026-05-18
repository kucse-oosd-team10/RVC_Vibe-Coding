#pragma once

#include "rvc/Types.hpp"

#include <chrono>
#include <functional>

namespace rvc {

class IFrontObstacleSensor {
  public:
    using InterruptHandler = std::function<void()>;

    virtual ~IFrontObstacleSensor() = default;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual void registerInterruptHandler(InterruptHandler handler) = 0;
};

class ISideObstacleSensor {
  public:
    virtual ~ISideObstacleSensor() = default;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual SideObstacleSnapshot read() = 0;
};

class IDustSensor {
  public:
    virtual ~IDustSensor() = default;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool isDustDetected() = 0;
};

class IObstacleSensor {
  public:
    virtual ~IObstacleSensor() = default;

    virtual bool initialize() = 0;
    virtual bool isFrontDetected() = 0;
    virtual bool isLeftDetected() = 0;
    virtual bool isRightDetected() = 0;
};

class IMotor {
  public:
    virtual ~IMotor() = default;

    virtual bool initialize() = 0;
    virtual void move(Direction direction) = 0;
};

class ICleaner {
  public:
    virtual ~ICleaner() = default;

    virtual bool initialize() = 0;
    virtual void setPower(PowerLevel level) = 0;
};

class IMovementMotor {
  public:
    virtual ~IMovementMotor() = default;

    virtual void stop() = 0;
    virtual void moveForward() = 0;
    virtual void moveBackward() = 0;
    virtual void turnLeft() = 0;
    virtual void turnRight() = 0;
};

class ICleaningMotor {
  public:
    virtual ~ICleaningMotor() = default;

    virtual void off() = 0;
    virtual void normal() = 0;
    virtual void powerUp() = 0;
};

class ITimer {
  public:
    virtual ~ITimer() = default;

    virtual void start(std::chrono::milliseconds duration) = 0;
    virtual bool expired() const = 0;
    virtual void reset() = 0;
};

} // namespace rvc
