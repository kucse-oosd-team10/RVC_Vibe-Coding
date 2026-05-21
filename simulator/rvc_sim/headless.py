"""Headless 실행 헬퍼: 시나리오 로드 → 러너 실행 → 어설션 평가."""
from __future__ import annotations

from pathlib import Path
from typing import Any, Dict, List, Tuple

from .assertions import evaluate_all
from .runner import RunResult, SimulationRunner
from .scenario import Scenario, load_scenario


def run_scenario(
    scenario: Scenario | str | Path,
    *,
    tick_duration_ms_override: int | None = None,
) -> Tuple[RunResult, List[Tuple[Dict[str, Any], str]]]:
    if not isinstance(scenario, Scenario):
        scenario = load_scenario(scenario)
    runner = SimulationRunner(scenario, tick_duration_ms_override=tick_duration_ms_override)
    result = runner.run()
    failures = evaluate_all(result)
    return result, failures


def run_and_check(
    scenario: Scenario | str | Path,
    *,
    tick_duration_ms_override: int | None = None,
) -> RunResult:
    """Run scenario and raise AssertionError if any assertion fails."""
    result, failures = run_scenario(
        scenario, tick_duration_ms_override=tick_duration_ms_override
    )
    if failures:
        lines = [f"Scenario {result.scenario.name!r} failed {len(failures)} assertion(s):"]
        for spec, err in failures:
            lines.append(f"  - [{spec.get('kind')}] {err}")
        raise AssertionError("\n".join(lines))
    return result
