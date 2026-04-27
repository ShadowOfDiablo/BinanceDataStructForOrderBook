#!/bin/bash
set -e

# Usage: ./benchmark.sh [--extended]
#   (default) normal mode  — 20 levels per side (Binance @depth20 cap)
#   --extended             — no depth cap, book grows to full market depth

if [[ "$1" == "--extended" ]]; then
    DEPTH_FLAG="-DEXTENDED_DEPTH=ON"
    echo "Building in extended mode (no depth cap)"
else
    DEPTH_FLAG="-DEXTENDED_DEPTH=OFF"
    echo "Building in normal mode (20 levels/side)"
fi

mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release $DEPTH_FLAG
make orderbook_bench -j$(nproc)
./benchmarks/orderbook_bench --benchmark_format=console
