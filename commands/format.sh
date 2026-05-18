#!/usr/bin/env bash

set -euo pipefail

CLANG_FORMAT_BIN="${CLANG_FORMAT:-clang-format}"

if ! command -v "${CLANG_FORMAT_BIN}" >/dev/null 2>&1 && [ -x /opt/homebrew/opt/llvm/bin/clang-format ]; then
    CLANG_FORMAT_BIN=/opt/homebrew/opt/llvm/bin/clang-format
fi

find include src tests -type f \( -name "*.cpp" -o -name "*.hpp" \) -print0 \
    | xargs -0 "${CLANG_FORMAT_BIN}" -i
