#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build/coverage"
PROFDATA="$BUILD_DIR/default.profdata"

echo "==> Configuring with coverage..."
cmake --preset coverage

echo "==> Building..."
cmake --build --preset coverage

echo "==> Running tests..."
LLVM_PROFILE_FILE="$BUILD_DIR/default_%p.profraw" \
    "$BUILD_DIR/tests/rvc_tests"

echo "==> Merging profile data..."
llvm-profdata merge -sparse "$BUILD_DIR"/default_*.profraw -o "$PROFDATA"

echo "==> Generating coverage report..."
llvm-cov show \
    "$BUILD_DIR/tests/rvc_tests" \
    -instr-profile="$PROFDATA" \
    -format=html \
    -output-dir=coverage-report \
    -ignore-filename-regex='build/|tests/'

echo "==> Generating lcov-style export for SonarCloud..."
llvm-cov export \
    "$BUILD_DIR/tests/rvc_tests" \
    -instr-profile="$PROFDATA" \
    -format=lcov \
    -ignore-filename-regex='build/|tests/' \
    > coverage-report/lcov.info

echo "==> Summary:"
llvm-cov report \
    "$BUILD_DIR/tests/rvc_tests" \
    -instr-profile="$PROFDATA" \
    -ignore-filename-regex='build/|tests/'

echo ""
echo "HTML report: coverage-report/index.html"
echo "LCOV export: coverage-report/lcov.info"

echo "==> Converting to SonarCloud generic format..."
python3 "$(dirname "$0")/lcov-to-sonar-generic.py" \
    coverage-report/lcov.info \
    coverage-report/sonar-coverage.xml
echo "SonarCloud XML: coverage-report/sonar-coverage.xml"