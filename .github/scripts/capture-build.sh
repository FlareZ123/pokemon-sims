#!/usr/bin/env bash
set -o pipefail
cmake --build "$1" --parallel 2 2>&1 | tee "$2"
