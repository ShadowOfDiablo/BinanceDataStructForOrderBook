#!/bin/bash
# Ensure build exists
if [ ! -f ./build/symbolbook_main ]; then
    ./build.sh
fi

# Ensure ws dependency is installed
if [ ! -d ./node_modules/ws ]; then
    npm install ws
fi

# Pipe Node.js output to C++ program
node websockets/index.js | ./build/symbolbook_main
