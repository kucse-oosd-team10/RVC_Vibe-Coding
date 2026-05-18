#!/usr/bin/env bash

set -euo pipefail

./commands/run-unit-tests.sh
./commands/run-system-tests.sh