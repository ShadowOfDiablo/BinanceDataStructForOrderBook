#!/bin/bash
set -e

# Ensure the C++ JSON header is present (downloaded by setup.sh)
if [ ! -f "orderBook/inc/json.hpp" ]; then
    ./setup.sh
fi

# Build the C++ engine (incremental — only recompiles changed files)
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make symbolbook_main -j$(nproc)
cd ..

# Feed stdin straight into the engine.
# Works three ways:
#   ./build.sh                     — interactive: paste JSON lines, Ctrl+D when done
#   cat sample.jsonl | ./build.sh  — pipe a file through
#   ./build.sh < sample.jsonl      — redirect a file in
./build/symbolbook_main
