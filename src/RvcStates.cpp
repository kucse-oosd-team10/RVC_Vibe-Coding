#include "rvc/RvcController.hpp"

namespace rvc {
namespace {

class OffState final : public IRvcState {
public:
    void startCleaning(RvcController& context) override {
        context.frontSensor().initialize();
        context.sideSensor().initialize();
        context.dustSensor().initialize();
        context.frontSensor().registerInterruptHandler([&context]() {
            context.onFrontObstacleDetected();
        });
        context.cleaningManager().start();
        context.movementManager().moveForward();
        context.changeState(makeForwardState());
    }

    [[nodiscard]] MovementState movementState() const override {
        return MovementState::Off;
    }
};

class ForwardState final : public IRvcState {
public:
    void onEnter(RvcController& context) override {
        context.cleaningManager().onMovementStateChanged(MovementState::Forward);
    }

    void onFrontObstacleDetected(RvcController& context) override {
        context.movementManager().stop();
        context.cleaningManager().onMovementStateChanged(MovementState::StoppedForObstacle);
        context.changeState(makeStoppedForObstacleState());
    }

    void onDustDetected(RvcController& context) override {
        context.cleaningManager().onDustDetected(MovementState::Forward);
    }

    [[nodiscard]] MovementState movementState() const override {
        return MovementState::Forward;
    }
};

class StoppedForObstacleState final : public IRvcState {
public:
    void onEnter(RvcController& context) override {
        sideSnapshot_ = context.sideSensor().read();
        const AvoidanceAction action =
            context.avoidanceStrategy().decideOnFrontObstacle(sideSnapshot_);

        if (action == AvoidanceAction::MoveBackward) {
            context.movementManager().moveBackward();
            context.cleaningManager().onMovementStateChanged(MovementState::Backward);
            context.changeState(makeBackwardState(sideSnapshot_));
            return;
        }

        if (action == AvoidanceAction::TurnRight) {
            context.movementManager().turnRight();
            context.cleaningManager().onMovementStateChanged(MovementState::TurningRight);
            context.changeState(makeTurningState(AvoidanceAction::TurnRight));
            return;
        }

        context.movementManager().turnLeft();
        context.cleaningManager().onMovementStateChanged(MovementState::TurningLeft);
        context.changeState(makeTurningState(AvoidanceAction::TurnLeft));
    }

    [[nodiscard]] MovementState movementState() const override {
        return MovementState::StoppedForObstacle;
    }

private:
    SideObstacleSnapshot sideSnapshot_{};
};

class TurningState final : public IRvcState {
public:
    explicit TurningState(AvoidanceAction turnAction) : turnAction_{turnAction} {
    }

    void tick(RvcController& context) override {
        if (!context.movementManager().isTurnComplete()) {
            return;
        }

        context.movementManager().moveForward();
        context.changeState(makeForwardState());
    }

    [[nodiscard]] MovementState movementState() const override {
        return turnAction_ == AvoidanceAction::TurnRight ? MovementState::TurningRight
                                                         : MovementState::TurningLeft;
    }

private:
    AvoidanceAction turnAction_;
};

class BackwardState final : public IRvcState {
public:
    explicit BackwardState(SideObstacleSnapshot initialSnapshot)
        : previousSideSnapshot_{initialSnapshot} {
    }

    void tick(RvcController& context) override {
        const SideObstacleSnapshot currentSideSnapshot = context.sideSensor().read();
        const AvoidanceAction action = context.avoidanceStrategy().decideWhileBackward(
            previousSideSnapshot_, currentSideSnapshot);
        previousSideSnapshot_ = currentSideSnapshot;

        if (action == AvoidanceAction::KeepBackward) {
            return;
        }

        if (action == AvoidanceAction::TurnRight) {
            context.movementManager().turnRight();
            context.cleaningManager().onMovementStateChanged(MovementState::TurningRight);
            context.changeState(makeTurningState(AvoidanceAction::TurnRight));
            return;
        }

        context.movementManager().turnLeft();
        context.cleaningManager().onMovementStateChanged(MovementState::TurningLeft);
        context.changeState(makeTurningState(AvoidanceAction::TurnLeft));
    }

    [[nodiscard]] MovementState movementState() const override {
        return MovementState::Backward;
    }

private:
    SideObstacleSnapshot previousSideSnapshot_;
};

} // namespace

std::unique_ptr<IRvcState> makeOffState() {
    return std::make_unique<OffState>();
}

std::unique_ptr<IRvcState> makeForwardState() {
    return std::make_unique<ForwardState>();
}

std::unique_ptr<IRvcState> makeStoppedForObstacleState() {
    return std::make_unique<StoppedForObstacleState>();
}

std::unique_ptr<IRvcState> makeTurningState(AvoidanceAction turnAction) {
    return std::make_unique<TurningState>(turnAction);
}

std::unique_ptr<IRvcState> makeBackwardState(SideObstacleSnapshot initialSnapshot) {
    return std::make_unique<BackwardState>(initialSnapshot);
}

} // namespace rvc
