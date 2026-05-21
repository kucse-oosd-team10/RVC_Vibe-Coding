#!/usr/bin/env bash

set -euo pipefail

SIMULATOR_ROOT="${RVC_SIMULATOR_ROOT:-$PWD/simulator}"
BINDINGS_DIR="${RVC_BINDINGS_DIR:-$PWD/build/bindings}"
SCENARIO="${1:-${SIMULATOR_ROOT}/scenarios/01_basic_cleaning.yaml}"

if [ ! -d "${SIMULATOR_ROOT}" ]; then
    echo "Simulator root not found: ${SIMULATOR_ROOT}"
    echo "Set RVC_SIMULATOR_ROOT to the existing simulator directory."
    exit 1
fi

if [ ! -d "${BINDINGS_DIR}" ]; then
    echo "Python bindings directory not found: ${BINDINGS_DIR}"
    echo "Run ./commands/configure.sh and ./commands/build.sh first, or set RVC_BINDINGS_DIR."
    exit 1
fi

PYTHONPATH="${BINDINGS_DIR}:${SIMULATOR_ROOT}${PYTHONPATH:+:${PYTHONPATH}}" \
    python3 -m rvc_sim.cli run "${SCENARIO}"
