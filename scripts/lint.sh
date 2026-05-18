#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build/dev}"
DIRS="src include"

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "Error: compile_commands.json not found in $BUILD_DIR"
    echo "Run 'cmake --preset dev' first."
    exit 1
fi

# macOS: pass SDK path so clang-tidy can find system headers
EXTRA_ARGS=""
if [[ "$(uname)" == "Darwin" ]]; then
    SDK_PATH="$(xcrun --show-sdk-path)"
    EXTRA_ARGS="--extra-arg=-isysroot --extra-arg=$SDK_PATH"
fi

FILES=$(find $DIRS -name '*.cpp')

echo "==> Running clang-tidy..."
echo "$FILES" | xargs clang-tidy -p "$BUILD_DIR" $EXTRA_ARGS
echo "==> clang-tidy passed."