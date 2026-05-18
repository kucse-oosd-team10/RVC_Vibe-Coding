#!/usr/bin/env bash

set -euo pipefail

cppcheck \
    --enable=all \
    --inconclusive \
    --std=c++20 \
    -Iinclude \
    --suppress=missingIncludeSystem \
    --suppress=unusedFunction \
    src include
