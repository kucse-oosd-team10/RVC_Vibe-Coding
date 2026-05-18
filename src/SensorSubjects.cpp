#include "rvc/SensorSubjects.hpp"

#include <algorithm>

namespace rvc {

ObstacleSensorSubject::ObstacleSensorSubject(IObstacleSensor &sensor) : sensor_{sensor} {}

void ObstacleSensorSubject::attach(ISensorObserver *observer) { observers_.push_back(observer); }

void ObstacleSensorSubject::detach(ISensorObserver *observer) {
    observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
}

void ObstacleSensorSubject::notify() {
    for (auto *observer : observers_) {
        if (observer != nullptr) {
            observer->onObstacleDetected(front_, left_, right_);
        }
    }
}

void ObstacleSensorSubject::poll() {
    front_ = sensor_.isFrontDetected();
    left_ = sensor_.isLeftDetected();
    right_ = sensor_.isRightDetected();
    if (front_) {
        notify();
    }
}

void ObstacleSensorSubject::onInterrupt() {
    front_ = true;
    left_ = sensor_.isLeftDetected();
    right_ = sensor_.isRightDetected();
    notify();
}

DustSensorSubject::DustSensorSubject(IDustSensor &sensor) : sensor_{sensor} {}

void DustSensorSubject::attach(ISensorObserver *observer) { observers_.push_back(observer); }

void DustSensorSubject::detach(ISensorObserver *observer) {
    observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
}

void DustSensorSubject::notify() {
    for (auto *observer : observers_) {
        if (observer != nullptr) {
            observer->onDustDetected(dustDetected_);
        }
    }
}

void DustSensorSubject::poll() {
    dustDetected_ = sensor_.isDustDetected();
    notify();
}

bool DustSensorSubject::isDustDetected() const { return dustDetected_; }

} // namespace rvc
