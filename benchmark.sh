#!/bin/bash
set -e

mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make orderbook_bench -j$(nproc)
./benchmarks/orderbook_bench --benchmark_format=console
