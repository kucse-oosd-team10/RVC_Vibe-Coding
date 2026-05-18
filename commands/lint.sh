#!/usr/bin/env bash

set -euo pipefail

CLANG_TIDY_BIN="${CLANG_TIDY:-clang-tidy}"

if ! command -v "${CLANG_TIDY_BIN}" >/dev/null 2>&1 && [ -x /opt/homebrew/opt/llvm/bin/clang-tidy ]; then
    CLANG_TIDY_BIN=/opt/homebrew/opt/llvm/bin/clang-tidy
fi

if ! find src tests -type f -name "*.cpp" | grep -q .; then
    echo "No C++ source files found."
    exit 0
fi

find src tests -type f -name "*.cpp" -exec "${CLANG_TIDY_BIN}" --quiet {} -- -Iinclude -std=c++20 \;
