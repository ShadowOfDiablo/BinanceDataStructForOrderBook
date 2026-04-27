#!/bin/bash
set -e

# Streams live Binance order book data via WebSocket and pipes it to the C++ engine.
# Run ./build.sh first to compile the binary.

if [ ! -f ./build/symbolbook_main ]; then
    echo "Binary not found — run ./build.sh first."
    exit 1
fi

if [ ! -d ./node_modules/ws ]; then
    npm install ws
fi

node websockets/index.js | ./build/symbolbook_main
