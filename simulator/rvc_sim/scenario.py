"""YAML 시나리오 로더.

스키마:

    name: <str>
    description: <str>
    tick_duration_ms: <int>      # 기본 100
    max_ticks: <int>
    world:
      size: [<width>, <height>]
      obstacles: [[x, y], ...]
      dust:
        - {pos: [x, y], amount: <float>}
      dust_threshold: <float>    # 기본 0.5
    robot:
      start: [x, y]
      heading: N | E | S | W
    cleaning_rates:
      off: 0.0
      normal: 0.5
      power_up: 2.0
    init_failures:
      motor: 0
      cleaner: 0
      obstacle_sensor: 0
      dust_sensor: 0
    power_off_at_tick: <int|null>   # 옵션
    assertions:
      - kind: <assertion_kind>
        ...
"""
from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import yaml


@dataclass
class WorldSpec:
    width: int
    height: int
    obstacles: List[Tuple[int, int]] = field(default_factory=list)
    dust: List[Tuple[Tuple[int, int], float]] = field(default_factory=list)
    dust_threshold: float = 0.5
    dust_replenish_interval_ticks: Optional[int] = None
    dust_replenish_amount: float = 1.0


@dataclass
class InterruptEvent:
    tick: int
    sub_phase: str = "before_poll"


@dataclass
class DustOverrideEvent:
    tick: int
    value: bool


@dataclass
class RobotSpec:
    start: Tuple[int, int]
    heading: str = "N"


@dataclass
class CleaningRates:
    off: float = 0.0
    normal: float = 0.5
    power_up: float = 2.0


@dataclass
class InitFailures:
    motor: int = 0
    cleaner: int = 0
    obstacle_sensor: int = 0
    dust_sensor: int = 0


@dataclass
class Scenario:
    name: str
    description: str
    tick_duration_ms: int
    max_ticks: int
    world: WorldSpec
    robot: RobotSpec
    cleaning_rates: CleaningRates
    init_failures: InitFailures
    assertions: List[Dict[str, Any]]
    power_off_at_tick: Optional[int] = None
    tail_ticks_after_power_off: int = 0
    interrupts: List[InterruptEvent] = field(default_factory=list)
    dust_sensor_override: List[DustOverrideEvent] = field(default_factory=list)
    source_path: Optional[Path] = None


def load_scenario(path: Path | str) -> Scenario:
    path = Path(path)
    with path.open("r") as f:
        data = yaml.safe_load(f)
    return _from_dict(data, source_path=path)


def _from_dict(data: Dict[str, Any], *, source_path: Optional[Path] = None) -> Scenario:
    world_data = data.get("world", {})
    size = world_data.get("size", [10, 10])
    obstacles = [tuple(o) for o in world_data.get("obstacles", [])]
    dust_entries: List[Tuple[Tuple[int, int], float]] = []
    for entry in world_data.get("dust", []) or []:
        pos = tuple(entry["pos"])
        amount = float(entry["amount"])
        dust_entries.append((pos, amount))
    replenish_interval = world_data.get("dust_replenish_interval_ticks")
    world = WorldSpec(
        width=int(size[0]),
        height=int(size[1]),
        obstacles=obstacles,
        dust=dust_entries,
        dust_threshold=float(world_data.get("dust_threshold", 0.5)),
        dust_replenish_interval_ticks=
            int(replenish_interval) if replenish_interval is not None else None,
        dust_replenish_amount=float(world_data.get("dust_replenish_amount", 1.0)),
    )

    robot_data = data.get("robot", {})
    robot = RobotSpec(
        start=tuple(robot_data.get("start", [0, 0])),
        heading=str(robot_data.get("heading", "N")),
    )

    rates_data = data.get("cleaning_rates", {})
    rates = CleaningRates(
        off=float(rates_data.get("off", 0.0)),
        normal=float(rates_data.get("normal", 0.5)),
        power_up=float(rates_data.get("power_up", 2.0)),
    )

    init_fail_data = data.get("init_failures", {})
    init_failures = InitFailures(
        motor=int(init_fail_data.get("motor", 0)),
        cleaner=int(init_fail_data.get("cleaner", 0)),
        obstacle_sensor=int(init_fail_data.get("obstacle_sensor", 0)),
        dust_sensor=int(init_fail_data.get("dust_sensor", 0)),
    )

    interrupts = [
        InterruptEvent(
            tick=int(entry["tick"]),
            sub_phase=str(entry.get("sub_phase", "before_poll")),
        )
        for entry in (data.get("interrupts") or [])
    ]

    dust_overrides = [
        DustOverrideEvent(tick=int(entry["tick"]), value=bool(entry["value"]))
        for entry in (data.get("dust_sensor_override") or [])
    ]

    return Scenario(
        name=str(data.get("name", source_path.stem if source_path else "unnamed")),
        description=str(data.get("description", "")),
        tick_duration_ms=int(data.get("tick_duration_ms", 100)),
        max_ticks=int(data.get("max_ticks", 50)),
        world=world,
        robot=robot,
        cleaning_rates=rates,
        init_failures=init_failures,
        assertions=list(data.get("assertions", []) or []),
        power_off_at_tick=data.get("power_off_at_tick"),
        tail_ticks_after_power_off=int(data.get("tail_ticks_after_power_off", 0)),
        interrupts=interrupts,
        dust_sensor_override=dust_overrides,
        source_path=source_path,
    )
