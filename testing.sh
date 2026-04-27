#!/bin/bash
set -e

mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make orderbook_tests -j$(nproc)
./tests/orderbook_tests
