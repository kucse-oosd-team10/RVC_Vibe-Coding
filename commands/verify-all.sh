#!/usr/bin/env bash
set -euo pipefail

./commands/format.sh
./commands/static-analysis.sh
./commands/build.sh
./commands/run-tests.sh
./commands/coverage.sh
./commands/sonarqube.sh