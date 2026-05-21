"""Scenario assertion engine.

Each assertion is a dict with at least ``kind: <str>``. Handlers consume the
spec dict and the ``RunResult`` and raise ``AssertionFailure`` on mismatch.

Supported kinds:

- ``state_at_tick``      ``{tick: int, expected: str}``
- ``final_state``        ``{expected: str}`` — state after powerOff
- ``motor_history_contains``  ``{direction: str}``
- ``motor_history_sequence``  ``{sequence: [str, ...]}``
- ``cleaner_power_at_tick``   ``{tick: int, expected: str}``
- ``cleaner_history_contains`` ``{power: str}``
- ``robot_pose_at_tick``      ``{tick: int, x: int, y: int, heading?: str}``
- ``robot_not_collided``      ``{}``
- ``robot_in_region``         ``{region: [[x1,y1],[x2,y2]]}``  inclusive bounds
- ``dust_remaining_lt``       ``{value: float}``
- ``dust_remaining_eq``       ``{value: float, tolerance?: float}``
- ``dust_collected_at_least`` ``{value: float}``
- ``collision_count_eq``      ``{value: int}``
"""
from __future__ import annotations

from typing import Any, Callable, Dict, List, Tuple

from .rvc import Direction, PowerLevel
from .runner import RunResult


class AssertionFailure(Exception):
    pass


HANDLERS: Dict[str, Callable[[Dict[str, Any], RunResult], None]] = {}


def register(kind: str):
    def decorator(fn):
        HANDLERS[kind] = fn
        return fn
    return decorator


def evaluate_all(result: RunResult) -> List[Tuple[Dict[str, Any], str]]:
    """Run every assertion. Return list of (spec, error_msg) for failures only."""
    failures: List[Tuple[Dict[str, Any], str]] = []
    for spec in result.scenario.assertions:
        kind = spec.get("kind")
        handler = HANDLERS.get(kind)
        if handler is None:
            failures.append((spec, f"unknown assertion kind {kind!r}"))
            continue
        try:
            handler(spec, result)
        except AssertionFailure as e:
            failures.append((spec, str(e)))
    return failures


def _snapshot_at(result: RunResult, tick: int):
    if tick < 0 or tick >= len(result.snapshots):
        raise AssertionFailure(
            f"tick {tick} out of range (snapshots: 0..{len(result.snapshots)-1})"
        )
    return result.snapshots[tick]


def _msg(spec: Dict[str, Any], detail: str) -> str:
    desc = spec.get("description")
    if desc:
        return f"{desc}: {detail}"
    return detail


@register("state_at_tick")
def _state_at_tick(spec, result):
    snap = _snapshot_at(result, int(spec["tick"]))
    expected = spec["expected"]
    if snap.state_name != expected:
        raise AssertionFailure(
            _msg(spec, f"state at tick {snap.tick} = {snap.state_name!r}, expected {expected!r}")
        )


@register("final_state")
def _final_state(spec, result):
    expected = spec["expected"]
    actual = result.state_after_power_off
    if actual != expected:
        raise AssertionFailure(_msg(spec, f"final state {actual!r} != {expected!r}"))


@register("motor_history_contains")
def _motor_contains(spec, result):
    direction = Direction.__members__[spec["direction"]]
    if direction not in result.motor_history:
        raise AssertionFailure(
            _msg(spec, f"motor history missing {direction.name}; got {[d.name for d in result.motor_history]}")
        )


@register("motor_history_sequence")
def _motor_sequence(spec, result):
    seq = [Direction.__members__[s] for s in spec["sequence"]]
    history = result.motor_history
    n = len(seq)
    for i in range(len(history) - n + 1):
        if history[i : i + n] == seq:
            return
    raise AssertionFailure(
        _msg(spec, f"sequence {[d.name for d in seq]} not found in motor history "
                   f"{[d.name for d in history]}")
    )


@register("cleaner_power_at_tick")
def _cleaner_power_at_tick(spec, result):
    snap = _snapshot_at(result, int(spec["tick"]))
    expected = PowerLevel.__members__[spec["expected"]]
    if snap.cleaner_power != expected:
        raise AssertionFailure(
            _msg(spec, f"cleaner power at tick {snap.tick} = {snap.cleaner_power.name}, "
                       f"expected {expected.name}")
        )


@register("cleaner_history_contains")
def _cleaner_history_contains(spec, result):
    power = PowerLevel.__members__[spec["power"]]
    if power not in result.cleaner_history:
        raise AssertionFailure(
            _msg(spec, f"cleaner history missing {power.name}; got "
                       f"{[p.name for p in result.cleaner_history]}")
        )


@register("robot_pose_at_tick")
def _robot_pose_at_tick(spec, result):
    snap = _snapshot_at(result, int(spec["tick"]))
    if snap.robot_x != int(spec["x"]) or snap.robot_y != int(spec["y"]):
        raise AssertionFailure(
            _msg(spec, f"robot at tick {snap.tick} = ({snap.robot_x},{snap.robot_y}), "
                       f"expected ({spec['x']},{spec['y']})")
        )
    if "heading" in spec and snap.robot_heading != spec["heading"]:
        raise AssertionFailure(
            _msg(spec, f"robot heading at tick {snap.tick} = {snap.robot_heading}, "
                       f"expected {spec['heading']}")
        )


@register("robot_not_collided")
def _robot_not_collided(spec, result):
    if result.collisions:
        raise AssertionFailure(
            _msg(spec, f"robot collided {len(result.collisions)} time(s) at {result.collisions}")
        )


@register("robot_in_region")
def _robot_in_region(spec, result):
    (x1, y1), (x2, y2) = spec["region"]
    xmin, xmax = sorted([int(x1), int(x2)])
    ymin, ymax = sorted([int(y1), int(y2)])
    for snap in result.snapshots:
        if not (xmin <= snap.robot_x <= xmax and ymin <= snap.robot_y <= ymax):
            raise AssertionFailure(
                _msg(spec, f"robot at tick {snap.tick} = ({snap.robot_x},{snap.robot_y}) "
                           f"outside region [{xmin},{xmax}]x[{ymin},{ymax}]")
            )


@register("dust_remaining_lt")
def _dust_remaining_lt(spec, result):
    threshold = float(spec["value"])
    if result.final_dust >= threshold:
        raise AssertionFailure(
            _msg(spec, f"dust remaining {result.final_dust} >= {threshold}")
        )


@register("dust_remaining_eq")
def _dust_remaining_eq(spec, result):
    expected = float(spec["value"])
    tol = float(spec.get("tolerance", 1e-6))
    if abs(result.final_dust - expected) > tol:
        raise AssertionFailure(
            _msg(spec, f"dust remaining {result.final_dust} != {expected} (tol={tol})")
        )


@register("dust_collected_at_least")
def _dust_collected_at_least(spec, result):
    expected = float(spec["value"])
    collected = result.initial_dust - result.final_dust
    if collected < expected:
        raise AssertionFailure(
            _msg(spec, f"dust collected {collected} < required {expected}")
        )


@register("collision_count_eq")
def _collision_count_eq(spec, result):
    expected = int(spec["value"])
    actual = len(result.collisions)
    if actual != expected:
        raise AssertionFailure(
            _msg(spec, f"collisions {actual} != {expected}")
        )


_AVOIDANCE_DIRECTIONS = {"LEFT", "RIGHT", "BACKWARD"}


def _first_avoidance_after(history_with_time, min_clock_ms: int):
    for direction, t in history_with_time:
        if t < min_clock_ms:
            continue
        if direction.name in _AVOIDANCE_DIRECTIONS:
            return direction, t
    return None


@register("interrupt_response_within_ticks")
def _interrupt_response_within_ticks(spec, result):
    """NFR-01: an avoidance motor command must follow the interrupt tick within `max_ticks`.

    AvoidingState.onEnter synchronously transitions back to Cleaning, so we
    detect avoidance by its observable side effect (LEFT/RIGHT/BACKWARD motor
    command) using the time-stamped motor history rather than the state name.
    """
    interrupt_tick = int(spec["interrupt_tick"])
    max_delay = int(spec["max_ticks"])

    trigger_clock_ms = None
    for snap in result.snapshots:
        if snap.tick == interrupt_tick:
            trigger_clock_ms = snap.clock_ms
            break
    if trigger_clock_ms is None:
        raise AssertionFailure(
            _msg(spec, f"interrupt tick {interrupt_tick} not found in snapshots")
        )

    deadline_ms = trigger_clock_ms + max_delay * result.tick_duration_ms
    hit = _first_avoidance_after(result.motor_history_with_time, trigger_clock_ms)
    if hit is None:
        raise AssertionFailure(
            _msg(spec, f"no avoidance motor command observed after interrupt tick "
                       f"{interrupt_tick} (clock={trigger_clock_ms}ms)")
        )
    direction, t = hit
    if t > deadline_ms:
        raise AssertionFailure(
            _msg(spec, f"first avoidance command {direction.name} at {t}ms exceeds "
                       f"deadline {deadline_ms}ms ({max_delay} tick(s) after interrupt)")
        )


@register("obstacle_to_avoid_within_ms")
def _obstacle_to_avoid_within_ms(spec, result):
    """NFR-02: an avoidance motor command must follow the trigger within `max_ms`."""
    trigger_tick = int(spec["trigger_tick"])
    max_ms = int(spec["max_ms"])

    trigger_clock_ms = None
    for snap in result.snapshots:
        if snap.tick == trigger_tick:
            trigger_clock_ms = snap.clock_ms
            break
    if trigger_clock_ms is None:
        raise AssertionFailure(_msg(spec, f"trigger tick {trigger_tick} not found in snapshots"))

    hit = _first_avoidance_after(result.motor_history_with_time, trigger_clock_ms)
    if hit is None:
        raise AssertionFailure(
            _msg(spec, f"no avoidance motor command observed after trigger tick {trigger_tick}")
        )
    direction, t = hit
    elapsed = t - trigger_clock_ms
    if elapsed > max_ms:
        raise AssertionFailure(
            _msg(spec, f"first avoidance {direction.name} at {t}ms (elapsed {elapsed}ms) "
                       f"exceeds {max_ms}ms after trigger tick {trigger_tick}")
        )


@register("cleaner_power_at_clock_ms")
def _cleaner_power_at_clock_ms(spec, result):
    """NFR-03: Cleaner power level at the snapshot whose clock_ms is closest to target."""
    target_ms = int(spec["clock_ms"])
    expected = PowerLevel.__members__[spec["expected"]]
    tolerance_ms = int(spec.get("tolerance_ms", 0))
    best = None
    for snap in result.snapshots:
        if best is None or abs(snap.clock_ms - target_ms) < abs(best.clock_ms - target_ms):
            best = snap
    if best is None:
        raise AssertionFailure(_msg(spec, "no snapshots available"))
    if abs(best.clock_ms - target_ms) > tolerance_ms:
        raise AssertionFailure(
            _msg(spec, f"closest snapshot at {best.clock_ms}ms is outside "
                       f"tolerance {tolerance_ms}ms of {target_ms}ms")
        )
    if best.cleaner_power != expected:
        raise AssertionFailure(
            _msg(spec, f"cleaner power at {best.clock_ms}ms = {best.cleaner_power.name}, "
                       f"expected {expected.name}")
        )


@register("motor_avoidance_count_eq")
def _motor_avoidance_count_eq(spec, result):
    expected = int(spec["value"])
    actual = sum(1 for d in result.motor_history if d.name in _AVOIDANCE_DIRECTIONS)
    if actual != expected:
        raise AssertionFailure(
            _msg(spec, f"avoidance commands {actual} != {expected} "
                       f"(history={[d.name for d in result.motor_history]})")
        )


@register("motor_avoidance_count_after_clock_ms_eq")
def _motor_avoidance_count_after_clock_ms_eq(spec, result):
    """Count LEFT/RIGHT/BACKWARD commands issued at clock >= threshold_ms."""
    threshold = int(spec["clock_ms"])
    expected = int(spec["value"])
    actual = sum(
        1 for d, t in result.motor_history_with_time
        if t >= threshold and d.name in _AVOIDANCE_DIRECTIONS
    )
    if actual != expected:
        raise AssertionFailure(
            _msg(spec, f"avoidance commands after {threshold}ms = {actual} != {expected}")
        )


@register("cleaner_power_count_eq")
def _cleaner_power_count_eq(spec, result):
    power = PowerLevel.__members__[spec["power"]]
    expected = int(spec["value"])
    actual = sum(1 for p in result.cleaner_history if p == power)
    if actual != expected:
        raise AssertionFailure(
            _msg(spec, f"cleaner {power.name} count {actual} != {expected} "
                       f"(history={[p.name for p in result.cleaner_history]})")
        )


@register("state_never_in")
def _state_never_in(spec, result):
    """NFR-07: Controller must never enter any of the forbidden states during the run."""
    forbidden = set(spec["states"])
    from_tick = int(spec.get("from_tick", 0))
    to_tick = spec.get("to_tick")
    for snap in result.snapshots:
        if snap.tick < from_tick:
            continue
        if to_tick is not None and snap.tick > int(to_tick):
            break
        if snap.state_name in forbidden:
            raise AssertionFailure(
                _msg(spec, f"forbidden state {snap.state_name!r} at tick {snap.tick}")
            )
