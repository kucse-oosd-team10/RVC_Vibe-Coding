"""Headless 시뮬레이션 러너 + Snapshot + FakeClock."""
from __future__ import annotations

from dataclasses import dataclass
from typing import List, Optional, Protocol

from .adapters import SimCleaner, SimDustSensor, SimMotor, SimObstacleSensor
from .robot import Robot
from .rvc import (
    CleaningManager,
    DefaultAvoidStrategy,
    Direction,
    DustSensorSubject,
    MovementManager,
    ObstacleSensorSubject,
    PowerLevel,
    RVCController,
    current_state_name,
)
from .scenario import CleaningRates, Scenario
from .types import Heading
from .world import World


class FakeClock:
    """Monotonic millisecond clock under runner's control."""

    def __init__(self, start_ms: int = 0) -> None:
        self.now_ms = start_ms

    def __call__(self) -> int:
        return self.now_ms

    def advance(self, ms: int) -> None:
        self.now_ms += ms


@dataclass
class Snapshot:
    tick: int
    clock_ms: int
    robot_x: int
    robot_y: int
    robot_heading: str
    state_name: str
    cleaner_power: PowerLevel
    motor_history_len: int
    last_motor_command: Optional[Direction]
    dust_total: float
    collision_count: int


@dataclass
class RunResult:
    scenario: Scenario
    snapshots: List[Snapshot]
    motor_history: List[Direction]
    motor_history_with_time: List[tuple]
    cleaner_history: List[PowerLevel]
    collisions: List[tuple]
    initial_dust: float
    final_dust: float
    state_after_power_off: str
    power_off_invoked: bool
    tick_duration_ms: int = 100


class GuiRenderer(Protocol):
    def render(self, runner: "SimulationRunner") -> bool: ...


def cleaning_rate_for(power: PowerLevel, rates: CleaningRates) -> float:
    return {
        PowerLevel.OFF: rates.off,
        PowerLevel.NORMAL: rates.normal,
        PowerLevel.POWER_UP: rates.power_up,
    }[power]


def build_world(scenario: Scenario) -> World:
    spec = scenario.world
    world = World(
        width=spec.width,
        height=spec.height,
        obstacles=set(spec.obstacles),
        dust=dict(spec.dust),
        dust_threshold=spec.dust_threshold,
    )
    if spec.dust_replenish_interval_ticks is not None:
        world.seed_replenish()
    return world


class SimulationRunner:
    def __init__(
        self,
        scenario: Scenario,
        *,
        gui: Optional[GuiRenderer] = None,
        tick_duration_ms_override: Optional[int] = None,
    ) -> None:
        self.scenario = scenario
        self.tick_duration_ms = tick_duration_ms_override or scenario.tick_duration_ms

        self.world = build_world(scenario)
        self.robot = Robot(
            scenario.robot.start[0],
            scenario.robot.start[1],
            Heading.from_label(scenario.robot.heading),
        )
        self.clock = FakeClock()

        self.motor = SimMotor(
            self.robot,
            self.world,
            init_fail_count=scenario.init_failures.motor,
            clock=self.clock,
        )
        self.cleaner = SimCleaner(init_fail_count=scenario.init_failures.cleaner)
        self.obs_sensor = SimObstacleSensor(
            self.world, self.robot, init_fail_count=scenario.init_failures.obstacle_sensor
        )
        self.dust_sensor = SimDustSensor(
            self.world, self.robot, init_fail_count=scenario.init_failures.dust_sensor
        )

        self.controller = self._build_controller()

        self.tick_count = 0
        self.snapshots: List[Snapshot] = []
        self.gui = gui
        self._initial_dust = self.world.total_dust()

        self._interrupts_by_tick: dict = {}
        for ev in scenario.interrupts:
            self._interrupts_by_tick.setdefault(ev.tick, []).append(ev)
        self._dust_override_by_tick: dict = {}
        for ev in scenario.dust_sensor_override:
            self._dust_override_by_tick.setdefault(ev.tick, []).append(ev)

    def _build_controller(self) -> RVCController:
        self.strategy = DefaultAvoidStrategy()
        self.movement_mgr = MovementManager(self.motor, self.strategy)
        self.cleaning_mgr = CleaningManager(self.cleaner, self.clock)
        self.obs_sub = ObstacleSensorSubject(self.obs_sensor)
        self.dust_sub = DustSensorSubject(self.dust_sensor)
        return RVCController(
            self.obs_sensor,
            self.dust_sensor,
            self.motor,
            self.cleaner,
            self.movement_mgr,
            self.cleaning_mgr,
            self.obs_sub,
            self.dust_sub,
        )

    def _snapshot(self) -> Snapshot:
        last_cmd = self.motor.history[-1] if self.motor.history else None
        return Snapshot(
            tick=self.tick_count,
            clock_ms=self.clock.now_ms,
            robot_x=self.robot.x,
            robot_y=self.robot.y,
            robot_heading=self.robot.heading.name,
            state_name=current_state_name(self.controller),
            cleaner_power=self.cleaner.current_power,
            motor_history_len=len(self.motor.history),
            last_motor_command=last_cmd,
            dust_total=self.world.total_dust(),
            collision_count=len(self.world.collisions),
        )

    def _dispatch_pre_tick_events(self, tick: int) -> None:
        """Fire events scheduled before controller.tick() polls sensors.

        - sub_phase=before_poll: calls onInterrupt() to mirror real ISR behavior.
        - dust override: pushes the value at the head of the override queue.
        """
        for ev in self._interrupts_by_tick.get(tick, []):
            if ev.sub_phase == "before_poll":
                self.obs_sub.onInterrupt()
        for ev in self._dust_override_by_tick.get(tick, []):
            self.dust_sensor.override_sequence.append(ev.value)

    def _dispatch_post_tick_events(self, tick: int) -> None:
        for ev in self._interrupts_by_tick.get(tick, []):
            if ev.sub_phase == "after_poll":
                self.obs_sub.onInterrupt()

    def run(self) -> RunResult:
        self.controller.powerOn()
        self.snapshots.append(self._snapshot())

        power_off_invoked = False
        tail_remaining = -1
        for _ in range(self.scenario.max_ticks):
            if (
                not power_off_invoked
                and self.scenario.power_off_at_tick is not None
                and self.tick_count == self.scenario.power_off_at_tick
            ):
                self.controller.powerOff()
                power_off_invoked = True
                tail_remaining = self.scenario.tail_ticks_after_power_off
                if tail_remaining <= 0:
                    break

            self.clock.advance(self.tick_duration_ms)
            self.motor.begin_tick()

            self._dispatch_pre_tick_events(self.tick_count)
            self.controller.tick()
            self._dispatch_post_tick_events(self.tick_count)
            self.motor.step_continuous()

            rate = cleaning_rate_for(self.cleaner.current_power, self.scenario.cleaning_rates)
            if rate > 0:
                self.world.collect_dust(self.robot.pose, rate)

            replenish = self.scenario.world.dust_replenish_interval_ticks
            if replenish and self.tick_count > 0 and self.tick_count % replenish == 0:
                self.world.replenish_dust(self.scenario.world.dust_replenish_amount)

            self.tick_count += 1
            self.snapshots.append(self._snapshot())

            if self.gui is not None:
                if not self.gui.render(self):
                    break

            if power_off_invoked:
                tail_remaining -= 1
                if tail_remaining <= 0:
                    break
        else:
            if not power_off_invoked:
                self.controller.powerOff()
                power_off_invoked = True

        if not power_off_invoked:
            self.controller.powerOff()
            power_off_invoked = True

        return RunResult(
            scenario=self.scenario,
            snapshots=list(self.snapshots),
            motor_history=list(self.motor.history),
            motor_history_with_time=list(self.motor.history_with_time),
            cleaner_history=list(self.cleaner.power_history),
            collisions=list(self.world.collisions),
            initial_dust=self._initial_dust,
            final_dust=self.world.total_dust(),
            state_after_power_off=current_state_name(self.controller),
            power_off_invoked=power_off_invoked,
            tick_duration_ms=self.tick_duration_ms,
        )
