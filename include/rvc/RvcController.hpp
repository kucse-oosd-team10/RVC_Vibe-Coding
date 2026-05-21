#pragma once

#include "rvc/CleaningManager.hpp"
#include "rvc/IObstacleAvoidanceStrategy.hpp"
#include "rvc/Interfaces.hpp"
#include "rvc/MovementManager.hpp"
#include "rvc/Types.hpp"

#include <memory>

namespace rvc {

class IRvcState {
public:
    virtual ~IRvcState() = default;

    virtual void onEnter(class RvcController& context);
    virtual void startCleaning(class RvcController& context);
    virtual void stopCleaning(class RvcController& context);
    virtual void onFrontObstacleDetected(class RvcController& context);
    virtual void onDustDetected(class RvcController& context);
    virtual void tick(class RvcController& context);
    [[nodiscard]] virtual MovementState movementState() const = 0;
};

class RvcController {
public:
    RvcController(IFrontObstacleSensor& frontSensor, ISideObstacleSensor& sideSensor,
                  IDustSensor& dustSensor, MovementManager& movementManager,
                  CleaningManager& cleaningManager, IObstacleAvoidanceStrategy& avoidanceStrategy);

    void startCleaning();
    void stopCleaning();
    void onFrontObstacleDetected();
    void onDustDetected();
    void tick();

    void changeState(std::unique_ptr<IRvcState> nextState);
    void performStop();

    [[nodiscard]] MovementState movementState() const;

    [[nodiscard]] IFrontObstacleSensor& frontSensor();
    [[nodiscard]] ISideObstacleSensor& sideSensor();
    [[nodiscard]] IDustSensor& dustSensor();
    [[nodiscard]] MovementManager& movementManager();
    [[nodiscard]] CleaningManager& cleaningManager();
    [[nodiscard]] IObstacleAvoidanceStrategy& avoidanceStrategy();

private:
    void enterState(std::unique_ptr<IRvcState> nextState);
    void enterPendingStateIfNeeded();

    IFrontObstacleSensor& frontSensor_;
    ISideObstacleSensor& sideSensor_;
    IDustSensor& dustSensor_;
    MovementManager& movementManager_;
    CleaningManager& cleaningManager_;
    IObstacleAvoidanceStrategy& avoidanceStrategy_;
    std::unique_ptr<IRvcState> currentState_;
    std::unique_ptr<IRvcState> pendingState_;
    bool enteringState_{false};
};

std::unique_ptr<IRvcState> makeOffState();
std::unique_ptr<IRvcState> makeForwardState();
std::unique_ptr<IRvcState> makeStoppedForObstacleState();
std::unique_ptr<IRvcState> makeTurningState(AvoidanceAction turnAction);
std::unique_ptr<IRvcState> makeBackwardState(SideObstacleSnapshot initialSnapshot);

} // namespace rvc
