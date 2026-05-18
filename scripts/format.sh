#!/usr/bin/env bash
set -euo pipefail

MODE="${1:-check}"
DIRS="src include tests"

FILES=$(find $DIRS -name '*.cpp' -o -name '*.hpp' -o -name '*.h')

if [ "$MODE" = "fix" ]; then
    echo "==> Formatting files in-place..."
    echo "$FILES" | xargs clang-format -i
    echo "==> Done."
elif [ "$MODE" = "check" ]; then
    echo "==> Checking formatting..."
    echo "$FILES" | xargs clang-format --dry-run --Werror
    echo "==> All files formatted correctly."
else
    echo "Usage: $0 [check|fix]"
    exit 1
fi