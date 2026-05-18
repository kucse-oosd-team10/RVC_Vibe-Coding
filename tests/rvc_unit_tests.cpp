#include "rvc/AdapterTimer.hpp"
#include "rvc/CleaningManager.hpp"
#include "rvc/IObstacleAvoidanceStrategy.hpp"
#include "rvc/LegacyAdapters.hpp"
#include "rvc/MovementManager.hpp"
#include "rvc/RvcController.hpp"
#include "rvc/SensorSubjects.hpp"
#include "rvc/SimulatorApi.hpp"

#include <gtest/gtest.h>

#include "TestDoubles.hpp"

using namespace rvc;
using namespace rvc::test;

class TestState final : public IRvcState {
public:
    explicit TestState(MovementState movementState) : movementState_{movementState} {
    }

    [[nodiscard]] MovementState movementState() const override {
        return movementState_;
    }

private:
    MovementState movementState_;
};

TEST(TimerTest, CoversLifecycleExpirationCallbackAndDefaultClock) {
    std::int64_t now = 100;
    int callbackCount = 0;
    Timer timer{[&now] {
        return now;
    }};

    EXPECT_FALSE(timer.isRunning());
    EXPECT_FALSE(timer.expired());

    timer.start(std::chrono::milliseconds{50});
    EXPECT_TRUE(timer.isRunning());
    EXPECT_FALSE(timer.expired());

    now = 149;
    EXPECT_EQ(now, 149);
    timer.update();
    EXPECT_EQ(callbackCount, 0);

    timer.setCallback([&callbackCount] {
        callbackCount += 1;
    });
    now = 150;
    EXPECT_EQ(now, 150);
    EXPECT_TRUE(timer.isExpired());
    timer.update();
    EXPECT_EQ(callbackCount, 1);
    EXPECT_FALSE(timer.isRunning());

    timer.update();
    EXPECT_EQ(callbackCount, 1);

    timer.setDuration(10);
    timer.start();
    now = 159;
    EXPECT_EQ(now, 159);
    EXPECT_FALSE(timer.expired());
    timer.stop();
    EXPECT_FALSE(timer.expired());

    timer.start();
    timer.reset();
    EXPECT_FALSE(timer.isRunning());
    EXPECT_FALSE(timer.isExpired());

    Timer defaultClockTimer;
    defaultClockTimer.setDuration(0);
    defaultClockTimer.start();
    EXPECT_TRUE(defaultClockTimer.expired());
}

TEST(TimerTest, UpdateWithoutCallbackStillStopsExpiredTimer) {
    std::int64_t now = 0;
    Timer timer{[&now] {
        return now;
    }};

    timer.start(std::chrono::milliseconds{5});
    now = 5;
    timer.update();

    EXPECT_FALSE(timer.isRunning());
    EXPECT_TRUE(timer.isExpired() == false);
}

TEST(TimerTest, RestartAllowsCallbackToFireAgain) {
    std::int64_t now = 10;
    int callbackCount = 0;
    Timer timer{[&now] {
        return now;
    }};
    timer.setDuration(3);
    timer.setCallback([&callbackCount] {
        callbackCount += 1;
    });

    timer.start();
    now = 13;
    EXPECT_EQ(now, 13);
    timer.update();
    EXPECT_EQ(callbackCount, 1);

    timer.start();
    now = 16;
    EXPECT_EQ(now, 16);
    timer.update();
    EXPECT_EQ(callbackCount, 2);
}

TEST(TimerTest, ResetBeforeExpirationPreventsCallback) {
    std::int64_t now = 0;
    int callbackCount = 0;
    Timer timer{[&now] {
        return now;
    }};
    timer.setCallback([&callbackCount] {
        callbackCount += 1;
    });

    timer.start(std::chrono::milliseconds{10});
    timer.reset();
    now = 100;
    timer.update();

    EXPECT_FALSE(timer.isRunning());
    EXPECT_EQ(callbackCount, 0);
}

TEST(LeftPriorityAvoidanceStrategyTest, DecidesTurnDirectionAndBackwardMovement) {
    const LeftPriorityAvoidanceStrategy strategy;

    EXPECT_EQ(strategy.decideOnFrontObstacle({false, false}), AvoidanceAction::TurnLeft);
    EXPECT_EQ(strategy.decideOnFrontObstacle({true, false}), AvoidanceAction::TurnRight);
    EXPECT_EQ(strategy.decideOnFrontObstacle({false, true}), AvoidanceAction::TurnLeft);
    EXPECT_EQ(strategy.decideOnFrontObstacle({true, true}), AvoidanceAction::MoveBackward);
}

TEST(LeftPriorityAvoidanceStrategyTest, DecidesDirectionWhileBackward) {
    const LeftPriorityAvoidanceStrategy strategy;

    EXPECT_EQ(strategy.decideWhileBackward({true, true}, {false, true}), AvoidanceAction::TurnLeft);
    EXPECT_EQ(strategy.decideWhileBackward({true, true}, {true, false}),
              AvoidanceAction::TurnRight);
    EXPECT_EQ(strategy.decideWhileBackward({true, true}, {false, false}),
              AvoidanceAction::TurnLeft);
    EXPECT_EQ(strategy.decideWhileBackward({true, true}, {true, true}),
              AvoidanceAction::KeepBackward);
}

TEST(LeftPriorityAvoidanceStrategyTest, TurnsLeftWhenOnlyRightObstacleExists) {
    const LeftPriorityAvoidanceStrategy strategy;

    EXPECT_EQ(strategy.decideOnFrontObstacle({false, true}), AvoidanceAction::TurnLeft);
}

TEST(LeftPriorityAvoidanceStrategyTest, LeftClearHasPriorityWhileBackward) {
    const LeftPriorityAvoidanceStrategy strategy;

    EXPECT_EQ(strategy.decideWhileBackward({true, true}, {false, false}),
              AvoidanceAction::TurnLeft);
}

TEST(LeftPriorityAvoidanceStrategyTest, KeepsBackwardWhenNoSideWasPreviouslyBlocked) {
    const LeftPriorityAvoidanceStrategy strategy;

    EXPECT_EQ(strategy.decideWhileBackward({false, false}, {false, false}),
              AvoidanceAction::KeepBackward);
}

TEST(DefaultAvoidStrategyTest, NeedsReverseRequiresFrontLeftAndRightTogether) {
    DefaultAvoidStrategy strategy;

    EXPECT_FALSE(strategy.needsReverse(false, true, true));
    EXPECT_FALSE(strategy.needsReverse(true, false, true));
    EXPECT_FALSE(strategy.needsReverse(true, true, false));
    EXPECT_TRUE(strategy.needsReverse(true, true, true));
}

TEST(DefaultAvoidStrategyTest, MapsSimulatorDecisionToDirection) {
    DefaultAvoidStrategy strategy;

    EXPECT_EQ(strategy.decideDirection(false, true, true), Direction::FORWARD);
    EXPECT_EQ(strategy.decideDirection(true, false, false), Direction::LEFT);
    EXPECT_EQ(strategy.decideDirection(true, false, true), Direction::LEFT);
    EXPECT_EQ(strategy.decideDirection(true, true, false), Direction::RIGHT);
    EXPECT_EQ(strategy.decideDirection(true, true, true), Direction::BACKWARD);
    EXPECT_FALSE(strategy.needsReverse(true, true, false));
    EXPECT_TRUE(strategy.needsReverse(true, true, true));
}

TEST(CleaningManagerTest, ExtendsPowerUpWhenDustIsStillDetected) {
    FakeCleaningMotor cleaningMotor;
    FakeDustSensor dustSensor;
    FakeTimer powerTimer;
    CleaningManager manager{cleaningMotor, dustSensor, powerTimer};

    manager.start();
    EXPECT_EQ(manager.currentState(), CleaningState::Normal);
    EXPECT_EQ(cleaningMotor.lastState(), CleaningState::Normal);

    manager.onDustDetected(MovementState::Forward);
    EXPECT_EQ(manager.currentState(), CleaningState::PowerUp);
    EXPECT_EQ(cleaningMotor.lastState(), CleaningState::PowerUp);

    dustSensor.setDustDetected(true);
    powerTimer.expire();
    manager.tick();
    EXPECT_EQ(manager.currentState(), CleaningState::PowerUp);
    EXPECT_EQ(powerTimer.startCount(), 2);

    dustSensor.setDustDetected(false);
    powerTimer.expire();
    manager.tick();
    EXPECT_EQ(manager.currentState(), CleaningState::Normal);
    EXPECT_EQ(cleaningMotor.lastState(), CleaningState::Normal);
}

TEST(CleaningManagerTest, DefersPowerUpWhenDustIsDetectedDuringAvoidance) {
    FakeCleaningMotor cleaningMotor;
    FakeDustSensor dustSensor;
    FakeTimer powerTimer;
    CleaningManager manager{cleaningMotor, dustSensor, powerTimer};

    manager.start();
    manager.onDustDetected(MovementState::Backward);
    EXPECT_EQ(manager.currentState(), CleaningState::Normal);
    EXPECT_TRUE(manager.pendingPowerUp());

    manager.onMovementStateChanged(MovementState::Forward);
    EXPECT_EQ(manager.currentState(), CleaningState::PowerUp);
    EXPECT_FALSE(manager.pendingPowerUp());
}

TEST(CleaningManagerTest, LegacyCleanerPathTracksDustAndPowerTransitions) {
    FakeCleaner cleaner;
    std::int64_t now = 0;
    CleaningManager manager{cleaner, [&now] {
                                return now;
                            }};

    manager.startCleaning();
    EXPECT_EQ(manager.currentState(), CleaningState::Normal);
    EXPECT_EQ(manager.getPowerLevel(), PowerLevel::NORMAL);
    EXPECT_EQ(cleaner.lastPower(), PowerLevel::NORMAL);

    manager.handleDustDetected(false);
    EXPECT_FALSE(manager.getLatestDustDetected());
    EXPECT_EQ(manager.currentState(), CleaningState::Normal);

    manager.handleDustDetected(true);
    EXPECT_TRUE(manager.getLatestDustDetected());
    EXPECT_EQ(manager.currentState(), CleaningState::PowerUp);
    EXPECT_EQ(manager.getPowerLevel(), PowerLevel::POWER_UP);

    now = 3000;
    EXPECT_EQ(now, 3000);
    manager.update();
    EXPECT_EQ(manager.currentState(), CleaningState::PowerUp);

    manager.handleDustDetected(false);
    now = 6000;
    EXPECT_EQ(now, 6000);
    manager.update();
    EXPECT_EQ(manager.currentState(), CleaningState::Normal);

    manager.powerUp();
    EXPECT_EQ(manager.currentState(), CleaningState::PowerUp);

    manager.stopCleaning();
    EXPECT_EQ(manager.currentState(), CleaningState::Off);
    EXPECT_EQ(manager.getPowerLevel(), PowerLevel::OFF);
    EXPECT_EQ(cleaner.lastPower(), PowerLevel::OFF);
}

TEST(CleaningManagerTest, IgnoresDustWhileOff) {
    FakeCleaningMotor cleaningMotor;
    FakeDustSensor dustSensor;
    FakeTimer powerTimer;
    CleaningManager manager{cleaningMotor, dustSensor, powerTimer};

    manager.onDustDetected(MovementState::Forward);

    EXPECT_EQ(manager.currentState(), CleaningState::Off);
    EXPECT_FALSE(manager.pendingPowerUp());
    EXPECT_TRUE(cleaningMotor.states.empty());
}

TEST(CleaningManagerTest, AvoidanceMovementForcesNormalFromPowerUp) {
    FakeCleaningMotor cleaningMotor;
    FakeDustSensor dustSensor;
    FakeTimer powerTimer;
    CleaningManager manager{cleaningMotor, dustSensor, powerTimer};

    manager.start();
    manager.onDustDetected(MovementState::Forward);
    EXPECT_EQ(manager.currentState(), CleaningState::PowerUp);

    manager.onMovementStateChanged(MovementState::TurningLeft);
    EXPECT_EQ(manager.currentState(), CleaningState::Normal);
    EXPECT_EQ(cleaningMotor.lastState(), CleaningState::Normal);
}

TEST(CleaningManagerTest, ForwardMovementWithoutPendingPowerUpRemainsNormal) {
    FakeCleaningMotor cleaningMotor;
    FakeDustSensor dustSensor;
    FakeTimer powerTimer;
    CleaningManager manager{cleaningMotor, dustSensor, powerTimer};

    manager.start();
    manager.onMovementStateChanged(MovementState::Forward);

    EXPECT_EQ(manager.currentState(), CleaningState::Normal);
    EXPECT_FALSE(manager.pendingPowerUp());
}

TEST(CleaningManagerTest, TickWhileNormalDoesNotPollDustSensor) {
    FakeCleaningMotor cleaningMotor;
    FakeDustSensor dustSensor;
    FakeTimer powerTimer;
    CleaningManager manager{cleaningMotor, dustSensor, powerTimer};

    manager.start();
    manager.tick();

    EXPECT_EQ(dustSensor.readCount, 0);
    EXPECT_EQ(manager.currentState(), CleaningState::Normal);
}

TEST(CleaningManagerTest, StopClearsPendingPowerUp) {
    FakeCleaningMotor cleaningMotor;
    FakeDustSensor dustSensor;
    FakeTimer powerTimer;
    CleaningManager manager{cleaningMotor, dustSensor, powerTimer};

    manager.start();
    manager.onDustDetected(MovementState::Backward);
    EXPECT_TRUE(manager.pendingPowerUp());

    manager.stop();
    manager.onMovementStateChanged(MovementState::Forward);

    EXPECT_FALSE(manager.pendingPowerUp());
    EXPECT_EQ(manager.currentState(), CleaningState::Off);
    EXPECT_EQ(cleaningMotor.lastState(), CleaningState::Off);
}

TEST(MovementManagerTest, ReportsTurnCompletionAfterTimerExpires) {
    FakeMovementMotor motor;
    FakeTimer turnTimer;
    MovementManager manager{motor, turnTimer};

    manager.turnLeft();
    EXPECT_EQ(manager.currentCommand(), MovementCommand::TurnLeft);
    EXPECT_FALSE(manager.isTurnComplete());

    turnTimer.expire();
    EXPECT_TRUE(manager.isTurnComplete());
}

TEST(MovementManagerTest, SimulatorMotorPathExecutesAllCommandsAndAvoidanceBranches) {
    FakeSimulatorMotor motor;
    DefaultAvoidStrategy strategy;
    MovementManager manager{motor, strategy};

    EXPECT_FALSE(manager.needsReverse(false, false, false));
    EXPECT_FALSE(manager.isTurnComplete());

    manager.moveForward();
    manager.stop();
    manager.moveBackward();
    manager.turnLeft();
    EXPECT_EQ(manager.currentCommand(), MovementCommand::TurnLeft);
    EXPECT_FALSE(manager.isTurnComplete());

    manager.turnRight();
    EXPECT_EQ(manager.currentCommand(), MovementCommand::TurnRight);
    manager.turn(Direction::RIGHT);
    manager.turn(Direction::LEFT);

    manager.executeAvoidance(true, true, true);
    EXPECT_EQ(manager.currentCommand(), MovementCommand::Backward);
    EXPECT_TRUE(manager.needsReverse(true, true, true));

    manager.executeAvoidance(true, true, false);
    EXPECT_EQ(manager.currentCommand(), MovementCommand::TurnRight);

    manager.executeAvoidance(true, false, false);
    EXPECT_EQ(manager.currentCommand(), MovementCommand::TurnLeft);

    const std::vector<Direction> expected{
        Direction::FORWARD, Direction::STOP,  Direction::BACKWARD, Direction::LEFT,
        Direction::RIGHT,   Direction::RIGHT, Direction::LEFT,     Direction::BACKWARD,
        Direction::RIGHT,   Direction::LEFT,
    };
    EXPECT_EQ(motor.commands, expected);
}

TEST(MovementManagerTest, DirectMotorPathSendsStopForwardAndBackward) {
    FakeMovementMotor motor;
    FakeTimer turnTimer;
    MovementManager manager{motor, turnTimer};

    manager.moveForward();
    manager.moveBackward();
    manager.stop();

    ASSERT_EQ(motor.commands.size(), 3U);
    EXPECT_EQ(motor.commands[0], MovementCommand::Forward);
    EXPECT_EQ(motor.commands[1], MovementCommand::Backward);
    EXPECT_EQ(motor.commands[2], MovementCommand::Stop);
    EXPECT_EQ(manager.currentCommand(), MovementCommand::Stop);
}

TEST(MovementManagerTest, DirectionTurnMapsLeftAndRight) {
    FakeMovementMotor motor;
    FakeTimer turnTimer;
    MovementManager manager{motor, turnTimer};

    manager.turn(Direction::LEFT);
    manager.turn(Direction::RIGHT);

    ASSERT_EQ(motor.commands.size(), 2U);
    EXPECT_EQ(motor.commands[0], MovementCommand::TurnLeft);
    EXPECT_EQ(motor.commands[1], MovementCommand::TurnRight);
}

TEST(MovementManagerTest, NonTurnCommandsAreNeverTurnComplete) {
    FakeMovementMotor motor;
    FakeTimer turnTimer;
    MovementManager manager{motor, turnTimer};

    manager.moveForward();
    turnTimer.expire();
    EXPECT_FALSE(manager.isTurnComplete());

    manager.moveBackward();
    turnTimer.expire();
    EXPECT_FALSE(manager.isTurnComplete());

    manager.stop();
    turnTimer.expire();
    EXPECT_FALSE(manager.isTurnComplete());
}

TEST(MovementManagerTest, CustomTurnDurationStartsTimerWithProvidedDuration) {
    FakeMovementMotor motor;
    FakeTimer turnTimer;
    MovementManager manager{motor, turnTimer, std::chrono::milliseconds{42}};

    manager.turnRight();

    EXPECT_EQ(turnTimer.duration(), std::chrono::milliseconds{42});
    EXPECT_EQ(turnTimer.startCount(), 1);
}

TEST(LegacyAdaptersTest, TranslateCombinedSensorAndMotorInterfaces) {
    FakeObstacleSensor obstacleSensor;
    obstacleSensor.setDetected(false, true, false);
    CombinedFrontObstacleSensorAdapter frontAdapter{obstacleSensor};
    CombinedSideObstacleSensorAdapter sideAdapter{obstacleSensor};

    bool interruptHandled = false;
    frontAdapter.registerInterruptHandler([&interruptHandled] {
        interruptHandled = true;
    });

    EXPECT_TRUE(frontAdapter.initialize());
    EXPECT_TRUE(sideAdapter.initialize());
    frontAdapter.triggerInterrupt();
    EXPECT_TRUE(interruptHandled);

    const SideObstacleSnapshot snapshot = sideAdapter.read();
    EXPECT_TRUE(snapshot.leftDetected);
    EXPECT_FALSE(snapshot.rightDetected);
    EXPECT_EQ(obstacleSensor.initializeCount, 2);

    FakeSimulatorMotor motor;
    MovementMotorAdapter movementAdapter{motor};
    movementAdapter.stop();
    movementAdapter.moveForward();
    movementAdapter.moveBackward();
    movementAdapter.turnLeft();
    movementAdapter.turnRight();

    ASSERT_EQ(motor.commands.size(), 5U);
    EXPECT_EQ(motor.commands[0], Direction::STOP);
    EXPECT_EQ(motor.commands[1], Direction::FORWARD);
    EXPECT_EQ(motor.commands[2], Direction::BACKWARD);
    EXPECT_EQ(motor.commands[3], Direction::LEFT);
    EXPECT_EQ(motor.commands[4], Direction::RIGHT);
}

TEST(LegacyAdaptersTest, TranslatesCleaningPowerLevels) {
    FakeCleaner cleaner;
    CleaningMotorAdapter adapter{cleaner};

    adapter.off();
    adapter.normal();
    adapter.powerUp();

    ASSERT_EQ(cleaner.levels.size(), 3U);
    EXPECT_EQ(cleaner.levels[0], PowerLevel::OFF);
    EXPECT_EQ(cleaner.levels[1], PowerLevel::NORMAL);
    EXPECT_EQ(cleaner.levels[2], PowerLevel::POWER_UP);
}

TEST(LegacyAdaptersTest, FrontAdapterTriggerWithoutHandlerIsSafe) {
    FakeObstacleSensor obstacleSensor;
    CombinedFrontObstacleSensorAdapter frontAdapter{obstacleSensor};

    frontAdapter.triggerInterrupt();
    frontAdapter.shutdown();

    EXPECT_EQ(obstacleSensor.initializeCount, 0);
}

TEST(LegacyAdaptersTest, SideAdapterReadsBothSideValues) {
    FakeObstacleSensor obstacleSensor;
    obstacleSensor.leftDetected = true;
    obstacleSensor.rightDetected = true;
    CombinedSideObstacleSensorAdapter sideAdapter{obstacleSensor};

    const SideObstacleSnapshot snapshot = sideAdapter.read();
    sideAdapter.shutdown();

    EXPECT_TRUE(snapshot.leftDetected);
    EXPECT_TRUE(snapshot.rightDetected);
    EXPECT_EQ(obstacleSensor.leftReadCount, 1);
    EXPECT_EQ(obstacleSensor.rightReadCount, 1);
}

TEST(LegacyAdaptersTest, AdapterInitializePropagatesSensorFailure) {
    FakeObstacleSensor obstacleSensor;
    obstacleSensor.initializeFailuresRemaining = 1;
    CombinedFrontObstacleSensorAdapter frontAdapter{obstacleSensor};

    EXPECT_FALSE(frontAdapter.initialize());
    EXPECT_TRUE(frontAdapter.initialize());
    EXPECT_EQ(obstacleSensor.initializeCount, 2);
}

class RecordingObserver final : public ISensorObserver {
public:
    void onObstacleDetected(bool front, bool left, bool right) override {
        obstacleEvents += 1;
        lastFront = front;
        lastLeft = left;
        lastRight = right;
    }

    void onDustDetected(bool detected) override {
        dustEvents += 1;
        lastDust = detected;
    }

    int obstacleEvents{0};
    int dustEvents{0};
    bool lastFront{false};
    bool lastLeft{false};
    bool lastRight{false};
    bool lastDust{false};
};

TEST(SensorSubjectsTest, NotifyOnlyFrontObstacleAndAlwaysPublishDustPolling) {
    FakeObstacleSensor obstacleSensor;
    ObstacleSensorSubject obstacleSubject{obstacleSensor};
    RecordingObserver observer;
    obstacleSubject.attach(&observer);

    obstacleSubject.poll();
    EXPECT_EQ(observer.obstacleEvents, 0);

    obstacleSensor.setDetected(true, true, false);
    obstacleSubject.poll();
    EXPECT_EQ(observer.obstacleEvents, 1);
    EXPECT_TRUE(observer.lastFront);
    EXPECT_TRUE(observer.lastLeft);
    EXPECT_FALSE(observer.lastRight);

    FakeDustSensor dustSensor;
    DustSensorSubject dustSubject{dustSensor};
    dustSubject.attach(&observer);

    dustSubject.poll();
    EXPECT_EQ(observer.dustEvents, 1);
    EXPECT_FALSE(observer.lastDust);

    dustSensor.setDustDetected(true);
    dustSubject.poll();
    EXPECT_EQ(observer.dustEvents, 2);
    EXPECT_TRUE(observer.lastDust);
}

TEST(SensorSubjectsTest, HandlesInterruptDetachNullObserverAndDustState) {
    FakeObstacleSensor obstacleSensor;
    obstacleSensor.leftDetected = false;
    obstacleSensor.rightDetected = true;
    ObstacleSensorSubject obstacleSubject{obstacleSensor};
    RecordingObserver observer;

    obstacleSubject.attach(nullptr);
    obstacleSubject.attach(&observer);
    obstacleSubject.onInterrupt();
    EXPECT_EQ(observer.obstacleEvents, 1);
    EXPECT_TRUE(observer.lastFront);
    EXPECT_FALSE(observer.lastLeft);
    EXPECT_TRUE(observer.lastRight);

    obstacleSubject.detach(&observer);
    obstacleSubject.onInterrupt();
    EXPECT_EQ(observer.obstacleEvents, 1);

    FakeDustSensor dustSensor;
    DustSensorSubject dustSubject{dustSensor};
    dustSubject.attach(nullptr);
    dustSubject.attach(&observer);
    EXPECT_FALSE(dustSubject.isDustDetected());

    dustSensor.setDustDetected(true);
    dustSubject.poll();
    EXPECT_TRUE(dustSubject.isDustDetected());
    EXPECT_EQ(observer.dustEvents, 1);

    dustSubject.detach(&observer);
    dustSensor.setDustDetected(false);
    dustSubject.poll();
    EXPECT_FALSE(dustSubject.isDustDetected());
    EXPECT_EQ(observer.dustEvents, 1);
}

TEST(SensorSubjectsTest, NotifiesMultipleObstacleObservers) {
    FakeObstacleSensor obstacleSensor;
    obstacleSensor.frontDetected = true;
    ObstacleSensorSubject obstacleSubject{obstacleSensor};
    RecordingObserver firstObserver;
    RecordingObserver secondObserver;
    obstacleSubject.attach(&firstObserver);
    obstacleSubject.attach(&secondObserver);

    obstacleSubject.poll();

    EXPECT_EQ(firstObserver.obstacleEvents, 1);
    EXPECT_EQ(secondObserver.obstacleEvents, 1);
    EXPECT_TRUE(firstObserver.lastFront);
    EXPECT_TRUE(secondObserver.lastFront);
}

TEST(SensorSubjectsTest, DirectDustNotifyUsesLatestPolledValue) {
    FakeDustSensor dustSensor;
    DustSensorSubject dustSubject{dustSensor};
    RecordingObserver observer;
    dustSubject.attach(&observer);

    dustSensor.setDustDetected(true);
    dustSubject.poll();
    dustSubject.notify();

    EXPECT_EQ(observer.dustEvents, 2);
    EXPECT_TRUE(observer.lastDust);
}

TEST(RvcControllerTest, StartStopInitializesAndShutsDownExternalInterfaces) {
    FakeFrontObstacleSensor frontSensor;
    FakeSideObstacleSensor sideSensor;
    FakeDustSensor dustSensor;
    FakeMovementMotor movementMotor;
    FakeCleaningMotor cleaningMotor;
    FakeTimer turnTimer;
    FakeTimer powerTimer;
    MovementManager movement{movementMotor, turnTimer};
    CleaningManager cleaning{cleaningMotor, dustSensor, powerTimer};
    LeftPriorityAvoidanceStrategy strategy;
    RvcController controller{frontSensor, sideSensor, dustSensor, movement, cleaning, strategy};

    controller.startCleaning();
    EXPECT_EQ(controller.movementState(), MovementState::Forward);
    EXPECT_TRUE(frontSensor.initialized);
    EXPECT_TRUE(sideSensor.initialized);
    EXPECT_TRUE(dustSensor.initialized);
    EXPECT_EQ(cleaning.currentState(), CleaningState::Normal);
    EXPECT_EQ(movement.currentCommand(), MovementCommand::Forward);

    controller.stopCleaning();
    EXPECT_EQ(controller.movementState(), MovementState::Off);
    EXPECT_FALSE(frontSensor.initialized);
    EXPECT_FALSE(sideSensor.initialized);
    EXPECT_FALSE(dustSensor.initialized);
    EXPECT_EQ(cleaning.currentState(), CleaningState::Off);
    EXPECT_EQ(movement.currentCommand(), MovementCommand::Stop);
}

TEST(RvcControllerTest, StartCommandIsIgnoredAfterAlreadyForward) {
    FakeFrontObstacleSensor frontSensor;
    FakeSideObstacleSensor sideSensor;
    FakeDustSensor dustSensor;
    FakeMovementMotor movementMotor;
    FakeCleaningMotor cleaningMotor;
    FakeTimer turnTimer;
    FakeTimer powerTimer;
    MovementManager movement{movementMotor, turnTimer};
    CleaningManager cleaning{cleaningMotor, dustSensor, powerTimer};
    LeftPriorityAvoidanceStrategy strategy;
    RvcController controller{frontSensor, sideSensor, dustSensor, movement, cleaning, strategy};

    controller.startCleaning();
    controller.startCleaning();

    EXPECT_EQ(controller.movementState(), MovementState::Forward);
    EXPECT_EQ(frontSensor.initializeCount, 1);
    EXPECT_EQ(movementMotor.commands.size(), 1U);
}

TEST(RvcControllerTest, StopWhileOffStillCommandsSafeShutdown) {
    FakeFrontObstacleSensor frontSensor;
    FakeSideObstacleSensor sideSensor;
    FakeDustSensor dustSensor;
    FakeMovementMotor movementMotor;
    FakeCleaningMotor cleaningMotor;
    FakeTimer turnTimer;
    FakeTimer powerTimer;
    MovementManager movement{movementMotor, turnTimer};
    CleaningManager cleaning{cleaningMotor, dustSensor, powerTimer};
    LeftPriorityAvoidanceStrategy strategy;
    RvcController controller{frontSensor, sideSensor, dustSensor, movement, cleaning, strategy};

    controller.stopCleaning();

    EXPECT_EQ(controller.movementState(), MovementState::Off);
    EXPECT_EQ(frontSensor.shutdownCount, 1);
    EXPECT_EQ(sideSensor.shutdownCount, 1);
    EXPECT_EQ(dustSensor.shutdownCount, 1);
    EXPECT_EQ(movement.currentCommand(), MovementCommand::Stop);
}

TEST(RvcControllerTest, FrontObstacleIsIgnoredWhileOff) {
    FakeFrontObstacleSensor frontSensor;
    FakeSideObstacleSensor sideSensor;
    FakeDustSensor dustSensor;
    FakeMovementMotor movementMotor;
    FakeCleaningMotor cleaningMotor;
    FakeTimer turnTimer;
    FakeTimer powerTimer;
    MovementManager movement{movementMotor, turnTimer};
    CleaningManager cleaning{cleaningMotor, dustSensor, powerTimer};
    LeftPriorityAvoidanceStrategy strategy;
    RvcController controller{frontSensor, sideSensor, dustSensor, movement, cleaning, strategy};

    controller.onFrontObstacleDetected();

    EXPECT_EQ(controller.movementState(), MovementState::Off);
    EXPECT_TRUE(movementMotor.commands.empty());
    EXPECT_EQ(sideSensor.readCount, 0);
}

TEST(RvcControllerTest, DustWhileForwardImmediatelyPowersUpCleaning) {
    FakeFrontObstacleSensor frontSensor;
    FakeSideObstacleSensor sideSensor;
    FakeDustSensor dustSensor;
    FakeMovementMotor movementMotor;
    FakeCleaningMotor cleaningMotor;
    FakeTimer turnTimer;
    FakeTimer powerTimer;
    MovementManager movement{movementMotor, turnTimer};
    CleaningManager cleaning{cleaningMotor, dustSensor, powerTimer};
    LeftPriorityAvoidanceStrategy strategy;
    RvcController controller{frontSensor, sideSensor, dustSensor, movement, cleaning, strategy};

    controller.startCleaning();
    controller.onDustDetected();

    EXPECT_EQ(controller.movementState(), MovementState::Forward);
    EXPECT_EQ(cleaning.currentState(), CleaningState::PowerUp);
    EXPECT_EQ(cleaningMotor.lastState(), CleaningState::PowerUp);
}

TEST(RvcControllerTest, FrontObstacleOnlyTurnsLeftThenMovesForward) {
    FakeFrontObstacleSensor frontSensor;
    FakeSideObstacleSensor sideSensor;
    FakeDustSensor dustSensor;
    FakeMovementMotor movementMotor;
    FakeCleaningMotor cleaningMotor;
    FakeTimer turnTimer;
    FakeTimer powerTimer;
    MovementManager movement{movementMotor, turnTimer};
    CleaningManager cleaning{cleaningMotor, dustSensor, powerTimer};
    LeftPriorityAvoidanceStrategy strategy;
    RvcController controller{frontSensor, sideSensor, dustSensor, movement, cleaning, strategy};

    controller.startCleaning();
    sideSensor.setCurrentSnapshot({false, false});
    frontSensor.triggerInterrupt();
    EXPECT_EQ(controller.movementState(), MovementState::TurningLeft);
    EXPECT_EQ(movement.currentCommand(), MovementCommand::TurnLeft);
    EXPECT_EQ(cleaning.currentState(), CleaningState::Normal);

    turnTimer.expire();
    controller.tick();
    EXPECT_EQ(controller.movementState(), MovementState::Forward);
    EXPECT_EQ(movement.currentCommand(), MovementCommand::Forward);
}

TEST(RvcControllerTest, FrontAndLeftObstacleTurnsRightThenMovesForward) {
    FakeFrontObstacleSensor frontSensor;
    FakeSideObstacleSensor sideSensor;
    FakeDustSensor dustSensor;
    FakeMovementMotor movementMotor;
    FakeCleaningMotor cleaningMotor;
    FakeTimer turnTimer;
    FakeTimer powerTimer;
    MovementManager movement{movementMotor, turnTimer};
    CleaningManager cleaning{cleaningMotor, dustSensor, powerTimer};
    LeftPriorityAvoidanceStrategy strategy;
    RvcController controller{frontSensor, sideSensor, dustSensor, movement, cleaning, strategy};

    controller.startCleaning();
    sideSensor.setCurrentSnapshot({true, false});
    frontSensor.triggerInterrupt();
    EXPECT_EQ(controller.movementState(), MovementState::TurningRight);
    EXPECT_EQ(movement.currentCommand(), MovementCommand::TurnRight);

    controller.tick();
    EXPECT_EQ(controller.movementState(), MovementState::TurningRight);
    EXPECT_EQ(movement.currentCommand(), MovementCommand::TurnRight);

    turnTimer.expire();
    controller.tick();
    EXPECT_EQ(controller.movementState(), MovementState::Forward);
    EXPECT_EQ(movement.currentCommand(), MovementCommand::Forward);
}

TEST(RvcControllerTest, BackwardKeepsMovingThenTurnsRightWhenRightSideClears) {
    FakeFrontObstacleSensor frontSensor;
    FakeSideObstacleSensor sideSensor;
    FakeDustSensor dustSensor;
    FakeMovementMotor movementMotor;
    FakeCleaningMotor cleaningMotor;
    FakeTimer turnTimer;
    FakeTimer powerTimer;
    MovementManager movement{movementMotor, turnTimer};
    CleaningManager cleaning{cleaningMotor, dustSensor, powerTimer};
    LeftPriorityAvoidanceStrategy strategy;
    RvcController controller{frontSensor, sideSensor, dustSensor, movement, cleaning, strategy};

    controller.startCleaning();
    sideSensor.setCurrentSnapshot({true, true});
    frontSensor.triggerInterrupt();
    EXPECT_EQ(controller.movementState(), MovementState::Backward);

    controller.tick();
    EXPECT_EQ(controller.movementState(), MovementState::Backward);
    EXPECT_EQ(movement.currentCommand(), MovementCommand::Backward);

    sideSensor.setCurrentSnapshot({true, false});
    controller.tick();
    EXPECT_EQ(controller.movementState(), MovementState::TurningRight);
    EXPECT_EQ(movement.currentCommand(), MovementCommand::TurnRight);
}

TEST(RvcControllerTest, BaseStateDefaultsAndFactoryStatesAreSafe) {
    FakeFrontObstacleSensor frontSensor;
    FakeSideObstacleSensor sideSensor;
    FakeDustSensor dustSensor;
    FakeMovementMotor movementMotor;
    FakeCleaningMotor cleaningMotor;
    FakeTimer turnTimer;
    FakeTimer powerTimer;
    MovementManager movement{movementMotor, turnTimer};
    CleaningManager cleaning{cleaningMotor, dustSensor, powerTimer};
    LeftPriorityAvoidanceStrategy strategy;
    RvcController controller{frontSensor, sideSensor, dustSensor, movement, cleaning, strategy};

    controller.tick();
    controller.onFrontObstacleDetected();
    controller.onDustDetected();
    EXPECT_EQ(controller.movementState(), MovementState::Off);
    EXPECT_EQ(cleaning.currentState(), CleaningState::Off);

    controller.changeState(makeStoppedForObstacleState());
    EXPECT_EQ(controller.movementState(), MovementState::TurningLeft);

    controller.changeState(makeTurningState(AvoidanceAction::TurnRight));
    EXPECT_EQ(controller.movementState(), MovementState::TurningRight);

    controller.changeState(makeBackwardState({true, true}));
    EXPECT_EQ(controller.movementState(), MovementState::Backward);

    auto stoppedState = makeStoppedForObstacleState();
    EXPECT_EQ(stoppedState->movementState(), MovementState::StoppedForObstacle);

    TestState defaultState{MovementState::Forward};
    defaultState.startCleaning(controller);
    defaultState.stopCleaning(controller);
    EXPECT_EQ(controller.movementState(), MovementState::Off);
}

TEST(RvcControllerTest, MockBasedObstacleEscapeScenarioIsCoveredByUnitTestOnly) {
    FakeFrontObstacleSensor frontSensor;
    FakeSideObstacleSensor sideSensor;
    FakeDustSensor dustSensor;
    FakeMovementMotor movementMotor;
    FakeCleaningMotor cleaningMotor;
    FakeTimer turnTimer;
    FakeTimer powerTimer;
    MovementManager movement{movementMotor, turnTimer};
    CleaningManager cleaning{cleaningMotor, dustSensor, powerTimer};
    LeftPriorityAvoidanceStrategy strategy;
    RvcController controller{frontSensor, sideSensor, dustSensor, movement, cleaning, strategy};

    controller.startCleaning();
    EXPECT_EQ(controller.movementState(), MovementState::Forward);

    sideSensor.setCurrentSnapshot({true, true});
    controller.onDustDetected();
    EXPECT_EQ(cleaning.currentState(), CleaningState::PowerUp);

    frontSensor.triggerInterrupt();
    EXPECT_EQ(controller.movementState(), MovementState::Backward);
    EXPECT_EQ(movement.currentCommand(), MovementCommand::Backward);

    controller.onDustDetected();
    EXPECT_EQ(cleaning.currentState(), CleaningState::Normal);
    EXPECT_TRUE(cleaning.pendingPowerUp());

    sideSensor.setCurrentSnapshot({false, true});
    controller.tick();
    EXPECT_EQ(controller.movementState(), MovementState::TurningLeft);

    turnTimer.expire();
    controller.tick();
    EXPECT_EQ(controller.movementState(), MovementState::Forward);
    EXPECT_EQ(cleaning.currentState(), CleaningState::PowerUp);

    dustSensor.setDustDetected(false);
    powerTimer.expire();
    controller.tick();
    EXPECT_EQ(cleaning.currentState(), CleaningState::Normal);
}

TEST(SimulatorRVCControllerAdapterTest, PowerOnInitializesDevicesAndStartsCoreController) {
    FakeObstacleSensor obstacleSensor;
    FakeDustSensor dustSensor;
    FakeSimulatorMotor motor;
    FakeCleaner cleaner;
    DefaultAvoidStrategy simulatorStrategy;
    MovementManager movementManager{motor, simulatorStrategy};
    CleaningManager cleaningManager{cleaner, [] {
                                        return 0;
                                    }};
    ObstacleSensorSubject obstacleSubject{obstacleSensor};
    DustSensorSubject dustSubject{dustSensor};
    RVCController controller{obstacleSensor,  dustSensor,      motor,           cleaner,
                             movementManager, cleaningManager, obstacleSubject, dustSubject};

    controller.powerOn();

    EXPECT_FALSE(controller.isError());
    EXPECT_EQ(current_state_name(controller), "Cleaning");
    EXPECT_EQ(motor.initializeCount, 1);
    EXPECT_EQ(cleaner.initializeCount, 1);
    EXPECT_GE(obstacleSensor.initializeCount, 1);
    EXPECT_GE(dustSensor.initializeCount, 1);
    ASSERT_FALSE(motor.commands.empty());
    EXPECT_EQ(motor.commands.back(), Direction::FORWARD);
    ASSERT_FALSE(cleaner.levels.empty());
    EXPECT_EQ(cleaner.levels.back(), PowerLevel::NORMAL);
}

TEST(SimulatorRVCControllerAdapterTest, TickPowerOffAndNonDetectedEventsRemainSafe) {
    FakeObstacleSensor obstacleSensor;
    FakeDustSensor dustSensor;
    FakeSimulatorMotor motor;
    FakeCleaner cleaner;
    DefaultAvoidStrategy simulatorStrategy;
    MovementManager movementManager{motor, simulatorStrategy};
    CleaningManager cleaningManager{cleaner, [] {
                                        return 0;
                                    }};
    ObstacleSensorSubject obstacleSubject{obstacleSensor};
    DustSensorSubject dustSubject{dustSensor};
    RVCController controller{obstacleSensor,  dustSensor,      motor,           cleaner,
                             movementManager, cleaningManager, obstacleSubject, dustSubject};

    controller.powerOn();
    const auto commandCountAfterPowerOn = motor.commands.size();
    const auto powerCountAfterPowerOn = cleaner.levels.size();

    controller.onObstacleDetected(false, true, true);
    controller.onDustDetected(false);
    controller.tick();
    EXPECT_FALSE(controller.isError());
    EXPECT_EQ(controller.movementState(), MovementState::Forward);
    EXPECT_EQ(motor.commands.size(), commandCountAfterPowerOn);
    EXPECT_EQ(cleaner.levels.size(), powerCountAfterPowerOn);

    controller.onDustDetected(true);
    EXPECT_EQ(cleaner.lastPower(), PowerLevel::POWER_UP);
    EXPECT_EQ(controller.coreController().movementState(), MovementState::Forward);
    const RVCController& constController = controller;
    EXPECT_EQ(constController.coreController().movementState(), MovementState::Forward);

    obstacleSensor.frontDetected = true;
    obstacleSensor.leftDetected = false;
    obstacleSensor.rightDetected = true;
    obstacleSubject.onInterrupt();
    EXPECT_EQ(current_state_name(controller), "Avoiding");
    EXPECT_EQ(motor.lastCommand(), Direction::LEFT);

    controller.powerOff();
    EXPECT_EQ(current_state_name(controller), "Off");
    EXPECT_EQ(motor.lastCommand(), Direction::STOP);
    EXPECT_EQ(cleaner.lastPower(), PowerLevel::OFF);

    controller.coreController().changeState(
        std::make_unique<TestState>(static_cast<MovementState>(255)));
    EXPECT_EQ(current_state_name(controller), "Unknown");
}

TEST(SimulatorRVCControllerAdapterTest, TickAndSensorEventsAreIgnoredWhileInError) {
    FakeObstacleSensor obstacleSensor;
    FakeDustSensor dustSensor;
    FakeSimulatorMotor motor;
    FakeCleaner cleaner;
    cleaner.initializeFailuresRemaining = 3;
    DefaultAvoidStrategy simulatorStrategy;
    MovementManager movementManager{motor, simulatorStrategy};
    CleaningManager cleaningManager{cleaner, [] {
                                        return 0;
                                    }};
    ObstacleSensorSubject obstacleSubject{obstacleSensor};
    DustSensorSubject dustSubject{dustSensor};
    RVCController controller{obstacleSensor,  dustSensor,      motor,           cleaner,
                             movementManager, cleaningManager, obstacleSubject, dustSubject};

    controller.powerOn();
    EXPECT_TRUE(controller.isError());
    const auto commandCountAfterError = motor.commands.size();
    const auto powerCountAfterError = cleaner.levels.size();

    obstacleSensor.frontDetected = true;
    dustSensor.dustDetected = true;
    controller.tick();
    controller.onObstacleDetected(true, false, false);
    controller.onDustDetected(true);

    EXPECT_TRUE(controller.isError());
    EXPECT_EQ(motor.commands.size(), commandCountAfterError);
    EXPECT_EQ(cleaner.levels.size(), powerCountAfterError);
}

TEST(SimulatorRVCControllerAdapterTest, PowerOnRetriesTransientInitializationFailures) {
    FakeObstacleSensor obstacleSensor;
    FakeDustSensor dustSensor;
    FakeSimulatorMotor motor;
    FakeCleaner cleaner;
    motor.initializeFailuresRemaining = 1;
    cleaner.initializeFailuresRemaining = 1;
    obstacleSensor.initializeFailuresRemaining = 1;
    DefaultAvoidStrategy simulatorStrategy;
    MovementManager movementManager{motor, simulatorStrategy};
    CleaningManager cleaningManager{cleaner, [] {
                                        return 0;
                                    }};
    ObstacleSensorSubject obstacleSubject{obstacleSensor};
    DustSensorSubject dustSubject{dustSensor};
    RVCController controller{obstacleSensor,  dustSensor,      motor,           cleaner,
                             movementManager, cleaningManager, obstacleSubject, dustSubject};

    controller.powerOn();

    EXPECT_FALSE(controller.isError());
    EXPECT_EQ(motor.initializeCount, 2);
    EXPECT_EQ(cleaner.initializeCount, 2);
    EXPECT_EQ(obstacleSensor.initializeCount, 4);
    EXPECT_EQ(dustSensor.initializeCount, 3);
    EXPECT_EQ(current_state_name(controller), "Cleaning");
}

TEST(SimulatorRVCControllerAdapterTest, TickPropagatesDustDetectionToCoreController) {
    FakeObstacleSensor obstacleSensor;
    FakeDustSensor dustSensor;
    FakeSimulatorMotor motor;
    FakeCleaner cleaner;
    DefaultAvoidStrategy simulatorStrategy;
    MovementManager movementManager{motor, simulatorStrategy};
    CleaningManager cleaningManager{cleaner, [] {
                                        return 0;
                                    }};
    ObstacleSensorSubject obstacleSubject{obstacleSensor};
    DustSensorSubject dustSubject{dustSensor};
    RVCController controller{obstacleSensor,  dustSensor,      motor,           cleaner,
                             movementManager, cleaningManager, obstacleSubject, dustSubject};

    controller.powerOn();
    dustSensor.dustDetected = true;
    controller.tick();

    EXPECT_EQ(cleaner.lastPower(), PowerLevel::POWER_UP);
    EXPECT_EQ(controller.movementState(), MovementState::Forward);
}

TEST(SimulatorRVCControllerAdapterTest, EntersErrorWhenDeviceInitializationRetriesAreExhausted) {
    FakeObstacleSensor obstacleSensor;
    FakeDustSensor dustSensor;
    FakeSimulatorMotor motor;
    FakeCleaner cleaner;
    motor.initializeFailuresRemaining = 3;
    DefaultAvoidStrategy simulatorStrategy;
    MovementManager movementManager{motor, simulatorStrategy};
    CleaningManager cleaningManager{cleaner, [] {
                                        return 0;
                                    }};
    ObstacleSensorSubject obstacleSubject{obstacleSensor};
    DustSensorSubject dustSubject{dustSensor};
    RVCController controller{obstacleSensor,  dustSensor,      motor,           cleaner,
                             movementManager, cleaningManager, obstacleSubject, dustSubject};

    controller.powerOn();

    EXPECT_TRUE(controller.isError());
    EXPECT_EQ(current_state_name(controller), "Error");
    EXPECT_EQ(motor.initializeCount, 3);
    ASSERT_FALSE(motor.commands.empty());
    EXPECT_EQ(motor.commands.back(), Direction::STOP);
    ASSERT_FALSE(cleaner.levels.empty());
    EXPECT_EQ(cleaner.levels.back(), PowerLevel::OFF);

    obstacleSensor.setDetected(true, false, false);
    obstacleSubject.onInterrupt();
    EXPECT_EQ(motor.commands.back(), Direction::STOP);

    controller.powerOff();
    EXPECT_FALSE(controller.isError());
    EXPECT_EQ(current_state_name(controller), "Off");
}
