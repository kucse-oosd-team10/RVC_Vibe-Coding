#!/usr/bin/env bash

set -euo pipefail

SIMULATOR_ROOT="${RVC_SIMULATOR_ROOT:-../rvc-controller/simulator}"
BINDINGS_DIR="${RVC_BINDINGS_DIR:-$PWD/build/bindings}"

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

PYTEST_BIN="${RVC_PYTEST:-pytest}"
PYTHONPATH="${BINDINGS_DIR}${PYTHONPATH:+:${PYTHONPATH}}" \
    "${PYTEST_BIN}" "${SIMULATOR_ROOT}/tests" -q -p no:cacheprovider
