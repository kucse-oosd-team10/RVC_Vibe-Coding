"""pybind11/Mock 인터페이스(IMotor, ICleaner, ...) 의 시뮬레이션 구현.

각 어댑터는 다음 두 가지 동작을 캡처:
- ``initialize()`` 실패 시나리오: ``init_fail_count`` 만큼 처음 N번 false 반환
- 외부 호출 history 기록 (시나리오 어설션용)
"""
from __future__ import annotations

from typing import Callable, List, Optional, Tuple

from .robot import Robot
from .rvc import (
    Direction,
    ICleaner,
    IDustSensor,
    IMotor,
    IObstacleSensor,
    PowerLevel,
)
from .types import Pose
from .world import World

ClockFn = Callable[[], int]


class SimMotor(IMotor):
    """Continuous-motion motor abstraction.

    Real RVC hardware treats ``motor.move(FORWARD)`` as a "keep moving forward
    until told otherwise" command — wheels spin continuously between calls. To
    match that semantic in a discrete-tick grid simulator:

    - ``move(FORWARD/BACKWARD)`` immediately advances 1 cell AND latches
      ``linear_motion`` for end-of-tick continuation.
    - ``move(LEFT/RIGHT)`` rotates immediately; ``linear_motion`` unchanged.
    - ``move(STOP)`` clears ``linear_motion``.
    - ``step_continuous()`` (called by runner end-of-tick) advances 1 cell along
      ``linear_motion`` only if no ``move()`` was issued this tick — preventing
      double-stepping on the tick a state transition issues a fresh command.
    """

    def __init__(
        self,
        robot: Robot,
        world: World,
        *,
        init_fail_count: int = 0,
        clock: Optional[ClockFn] = None,
    ) -> None:
        super().__init__()
        self._robot = robot
        self._world = world
        self._clock = clock
        self.history: List[Direction] = []
        self.history_with_time: List[Tuple[Direction, int]] = []
        self._init_fail_remaining = init_fail_count
        self._linear_motion: Direction = Direction.STOP
        self._commanded_this_tick: bool = False
        self.tick_pose_log: List[Pose] = []

    @property
    def linear_motion(self) -> Direction:
        return self._linear_motion

    def initialize(self) -> bool:
        if self._init_fail_remaining > 0:
            self._init_fail_remaining -= 1
            return False
        return True

    def begin_tick(self) -> None:
        self.tick_pose_log.clear()

    def move(self, direction: Direction) -> None:
        self.history.append(direction)
        now_ms = self._clock() if self._clock is not None else 0
        self.history_with_time.append((direction, now_ms))
        self._commanded_this_tick = True
        if direction in (Direction.FORWARD, Direction.BACKWARD):
            self._linear_motion = direction
            self._robot.move(direction, self._world)
        elif direction in (Direction.LEFT, Direction.RIGHT):
            self._robot.move(direction, self._world)
        elif direction == Direction.STOP:
            self._linear_motion = Direction.STOP
        self.tick_pose_log.append(self._robot.pose)

    def step_continuous(self) -> bool:
        moved = False
        if (
            not self._commanded_this_tick
            and self._linear_motion in (Direction.FORWARD, Direction.BACKWARD)
        ):
            self._robot.move(self._linear_motion, self._world)
            self.tick_pose_log.append(self._robot.pose)
            moved = True
        self._commanded_this_tick = False
        return moved


class SimCleaner(ICleaner):
    def __init__(self, *, init_fail_count: int = 0) -> None:
        super().__init__()
        self.power_history: List[PowerLevel] = []
        self.current_power: PowerLevel = PowerLevel.OFF
        self._init_fail_remaining = init_fail_count

    def initialize(self) -> bool:
        if self._init_fail_remaining > 0:
            self._init_fail_remaining -= 1
            return False
        return True

    def setPower(self, level: PowerLevel) -> None:
        self.current_power = level
        self.power_history.append(level)


class SimObstacleSensor(IObstacleSensor):
    """Obstacle sensor with interrupt-mode override.

    `force_front_reads` mirrors the real interrupt path: while non-zero, the
    next N isFrontDetected() calls return True regardless of world state.
    Set to 2 by the runner to cover both ObstacleSensorSubject.onInterrupt()
    (one read) and the immediately-following poll() (one more read) inside the
    same controller.tick(), so the forced state survives long enough for the
    state machine to act on it.
    """

    def __init__(self, world: World, robot: Robot, *, init_fail_count: int = 0) -> None:
        super().__init__()
        self._world = world
        self._robot = robot
        self._init_fail_remaining = init_fail_count
        self.force_front_reads: int = 0

    def initialize(self) -> bool:
        if self._init_fail_remaining > 0:
            self._init_fail_remaining -= 1
            return False
        return True

    def arm_front_interrupt(self, reads: int = 2) -> None:
        self.force_front_reads = max(self.force_front_reads, reads)

    def isFrontDetected(self) -> bool:
        if self.force_front_reads > 0:
            self.force_front_reads -= 1
            return True
        return self._world.is_blocked(self._robot.pose.front_pos())

    def isLeftDetected(self) -> bool:
        return self._world.is_blocked(self._robot.pose.left_pos())

    def isRightDetected(self) -> bool:
        return self._world.is_blocked(self._robot.pose.right_pos())


class SimDustSensor(IDustSensor):
    """Dust sensor with optional override sequence.

    `override_sequence` is a list of bool consumed once-per-poll (FIFO). When
    exhausted, falls back to world.detect_dust(robot.pose). Used by NFR-03 to
    drive POWER_UP timing without needing exact world dust placement.
    """

    def __init__(self, world: World, robot: Robot, *, init_fail_count: int = 0) -> None:
        super().__init__()
        self._world = world
        self._robot = robot
        self._init_fail_remaining = init_fail_count
        self.override_sequence: List[bool] = []

    def initialize(self) -> bool:
        if self._init_fail_remaining > 0:
            self._init_fail_remaining -= 1
            return False
        return True

    def isDustDetected(self) -> bool:
        if self.override_sequence:
            return self.override_sequence.pop(0)
        return self._world.detect_dust(self._robot.pose)
