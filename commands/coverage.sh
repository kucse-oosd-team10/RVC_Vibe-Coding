#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build/coverage"

cmake -S . -B "$BUILD_DIR" \
  -G Ninja \
  -U BUILD_PYTHON_BINDINGS \
  -U BUILD_UNIT_TESTS \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DBUILD_PYTHON_BINDINGS=OFF \
  -DBUILD_UNIT_TESTS=ON \
  -DCMAKE_CXX_FLAGS="--coverage" \
  -DCMAKE_EXE_LINKER_FLAGS="--coverage"

cmake --build "$BUILD_DIR"

ctest --test-dir "$BUILD_DIR" --output-on-failure

gcovr \
  --root . \
  --xml-pretty \
  --output "$BUILD_DIR/coverage.xml" \
  --filter 'src/.*' \
  --filter 'include/.*' \
  --exclude 'tests/.*' \
  --exclude 'simulator/.*' \
  --exclude 'build/.*' \
  --exclude 'bindings/.*'
