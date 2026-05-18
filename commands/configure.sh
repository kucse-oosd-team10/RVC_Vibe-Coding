#!/usr/bin/env bash

set -euo pipefail

cmake -S . -B build \
    -G Ninja \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON