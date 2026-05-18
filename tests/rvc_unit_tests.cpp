#include "TestDoubles.hpp"
#include "rvc/CleaningManager.hpp"
#include "rvc/IObstacleAvoidanceStrategy.hpp"
#include "rvc/LegacyAdapters.hpp"
#include "rvc/MovementManager.hpp"
#include "rvc/RvcController.hpp"
#include "rvc/SensorSubjects.hpp"
#include "rvc/SimulatorApi.hpp"

#include <gtest/gtest.h>

using namespace rvc;
using namespace rvc::test;

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

    dustSensor.dustDetected = true;
    powerTimer.expire();
    manager.tick();
    EXPECT_EQ(manager.currentState(), CleaningState::PowerUp);
    EXPECT_EQ(powerTimer.startCount(), 2);

    dustSensor.dustDetected = false;
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

TEST(LegacyAdaptersTest, TranslateCombinedSensorAndMotorInterfaces) {
    FakeObstacleSensor obstacleSensor;
    obstacleSensor.leftDetected = true;
    obstacleSensor.rightDetected = false;
    CombinedFrontObstacleSensorAdapter frontAdapter{obstacleSensor};
    CombinedSideObstacleSensorAdapter sideAdapter{obstacleSensor};

    bool interruptHandled = false;
    frontAdapter.registerInterruptHandler([&interruptHandled] { interruptHandled = true; });

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

    obstacleSensor.frontDetected = true;
    obstacleSensor.leftDetected = true;
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

    dustSensor.dustDetected = true;
    dustSubject.poll();
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
    sideSensor.currentSnapshot = {false, false};
    frontSensor.triggerInterrupt();
    EXPECT_EQ(controller.movementState(), MovementState::TurningLeft);
    EXPECT_EQ(movement.currentCommand(), MovementCommand::TurnLeft);
    EXPECT_EQ(cleaning.currentState(), CleaningState::Normal);

    turnTimer.expire();
    controller.tick();
    EXPECT_EQ(controller.movementState(), MovementState::Forward);
    EXPECT_EQ(movement.currentCommand(), MovementCommand::Forward);
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

    sideSensor.currentSnapshot = {true, true};
    frontSensor.triggerInterrupt();
    EXPECT_EQ(controller.movementState(), MovementState::Backward);
    EXPECT_EQ(movement.currentCommand(), MovementCommand::Backward);

    controller.onDustDetected();
    EXPECT_EQ(cleaning.currentState(), CleaningState::Normal);
    EXPECT_TRUE(cleaning.pendingPowerUp());

    sideSensor.currentSnapshot = {false, true};
    controller.tick();
    EXPECT_EQ(controller.movementState(), MovementState::TurningLeft);

    turnTimer.expire();
    controller.tick();
    EXPECT_EQ(controller.movementState(), MovementState::Forward);
    EXPECT_EQ(cleaning.currentState(), CleaningState::PowerUp);

    dustSensor.dustDetected = false;
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
    CleaningManager cleaningManager{cleaner, [] { return 0; }};
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

TEST(SimulatorRVCControllerAdapterTest, EntersErrorWhenDeviceInitializationRetriesAreExhausted) {
    FakeObstacleSensor obstacleSensor;
    FakeDustSensor dustSensor;
    FakeSimulatorMotor motor;
    FakeCleaner cleaner;
    motor.initializeFailuresRemaining = 3;
    DefaultAvoidStrategy simulatorStrategy;
    MovementManager movementManager{motor, simulatorStrategy};
    CleaningManager cleaningManager{cleaner, [] { return 0; }};
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

    obstacleSensor.frontDetected = true;
    obstacleSubject.onInterrupt();
    EXPECT_EQ(motor.commands.back(), Direction::STOP);

    controller.powerOff();
    EXPECT_FALSE(controller.isError());
    EXPECT_EQ(current_state_name(controller), "Off");
}
