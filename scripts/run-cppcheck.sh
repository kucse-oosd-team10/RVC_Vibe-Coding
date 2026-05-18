#!/usr/bin/env bash
set -euo pipefail

SRC_DIRS="src include tests"

echo "==> Running cppcheck..."
cppcheck \
    --enable=all \
    --std=c++20 \
    --language=c++ \
    --suppress=missingIncludeSystem \
    --suppress=unmatchedSuppression \
    --inline-suppr \
    -I include \
    --xml \
    $SRC_DIRS \
    2> cppcheck-report.xml

echo "==> cppcheck report: cppcheck-report.xml"

# Also print human-readable summary
cppcheck \
    --enable=all \
    --std=c++20 \
    --language=c++ \
    --suppress=missingIncludeSystem \
    --suppress=unmatchedSuppression \
    --inline-suppr \
    -I include \
    $SRC_DIRS \
    2>&1 || true