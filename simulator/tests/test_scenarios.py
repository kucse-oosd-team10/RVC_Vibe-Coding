"""Parametrized scenario test: every YAML in scenarios/ must pass its assertions."""
from __future__ import annotations

from pathlib import Path

import pytest
import yaml

from rvc_sim.headless import run_scenario

from conftest import discover_scenarios


def _scenario_is_slow(path: Path) -> bool:
    with path.open("r") as f:
        data = yaml.safe_load(f) or {}
    return bool(data.get("slow", False))


SCENARIO_FILES = discover_scenarios()
PARAMS = [
    pytest.param(
        path,
        id=path.stem,
        marks=pytest.mark.slow if _scenario_is_slow(path) else (),
    )
    for path in SCENARIO_FILES
]


@pytest.mark.parametrize("scenario_path", PARAMS)
def test_scenario(scenario_path: Path) -> None:
    result, failures = run_scenario(scenario_path)
    assert not failures, (
        f"Scenario {scenario_path.name} failed:\n"
        + "\n".join(f"  - [{spec.get('kind')}] {err}" for spec, err in failures)
    )
    assert result.power_off_invoked, "powerOff was not called by the runner"
