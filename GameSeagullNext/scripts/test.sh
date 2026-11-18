#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build artifacts
"${CXX:-g++}" -std=c++17 -O2 -Wall -Wextra -Werror core/*.cpp tests/test_core.cpp -o build/test_core
./build/test_core 2>artifacts/test-diagnostics.json | tee artifacts/test-results.txt
