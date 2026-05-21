"""rvc-sim CLI entrypoint."""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

from .assertions import evaluate_all
from .runner import SimulationRunner
from .scenario import load_scenario


DEFAULT_GUI_TICK_MS = 500


def _cmd_run(args: argparse.Namespace) -> int:
    scenario = load_scenario(args.scenario)
    tick_duration_ms = args.tick_ms
    gui = None
    if args.gui:
        from .gui import PygameRenderer  # imported lazily to keep headless light

        gui = PygameRenderer()
        if tick_duration_ms is None:
            tick_duration_ms = DEFAULT_GUI_TICK_MS
    runner = SimulationRunner(
        scenario,
        gui=gui,
        tick_duration_ms_override=tick_duration_ms,
    )
    result = runner.run()
    failures = evaluate_all(result)

    print(f"scenario: {result.scenario.name}")
    print(f"ticks executed: {result.snapshots[-1].tick}")
    print(f"final state (after powerOff): {result.state_after_power_off}")
    print(f"dust collected: {result.initial_dust - result.final_dust:.3f} "
          f"(initial {result.initial_dust:.3f} -> final {result.final_dust:.3f})")
    print(f"collisions: {len(result.collisions)}")
    print(f"motor commands: {len(result.motor_history)}")

    if failures:
        print(f"\nFAILED ({len(failures)} assertion(s)):")
        for spec, err in failures:
            print(f"  - [{spec.get('kind')}] {err}")
        return 1
    print("\nPASSED (all assertions)")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="rvc-sim", description="RVC simulator")
    sub = parser.add_subparsers(dest="cmd", required=True)

    p_run = sub.add_parser("run", help="run a single scenario file")
    p_run.add_argument("scenario", type=Path, help="path to .yaml scenario")
    p_run.add_argument("--gui", action="store_true", help="enable pygame GUI")
    p_run.add_argument("--tick-ms", type=int, default=None,
                       help=("override actual tick_duration_ms; GUI defaults to "
                             f"{DEFAULT_GUI_TICK_MS}ms when omitted"))
    p_run.set_defaults(func=_cmd_run)

    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
