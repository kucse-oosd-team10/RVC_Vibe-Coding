#pragma once

#include "rvc/Interfaces.hpp"

#include <vector>

namespace rvc {

class ISensorObserver {
public:
    virtual ~ISensorObserver() = default;
    virtual void onObstacleDetected(bool front, bool left, bool right) = 0;
    virtual void onDustDetected(bool detected) = 0;
};

class ISensorSubject {
public:
    virtual ~ISensorSubject() = default;
    virtual void attach(ISensorObserver* observer) = 0;
    virtual void detach(ISensorObserver* observer) = 0;
    virtual void notify() = 0;
};

class ObstacleSensorSubject final : public ISensorSubject {
public:
    explicit ObstacleSensorSubject(IObstacleSensor& sensor);

    void attach(ISensorObserver* observer) override;
    void detach(ISensorObserver* observer) override;
    void notify() override;
    void poll();
    void onInterrupt();

private:
    IObstacleSensor& sensor_;
    std::vector<ISensorObserver*> observers_;
    bool front_{false};
    bool left_{false};
    bool right_{false};
};

class DustSensorSubject final : public ISensorSubject {
public:
    explicit DustSensorSubject(IDustSensor& sensor);

    void attach(ISensorObserver* observer) override;
    void detach(ISensorObserver* observer) override;
    void notify() override;
    void poll();
    [[nodiscard]] bool isDustDetected() const;

private:
    IDustSensor& sensor_;
    std::vector<ISensorObserver*> observers_;
    bool dustDetected_{false};
};

} // namespace rvc
