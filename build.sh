#!/bin/bash
set -e

# Run setup to ensure tools and dependencies are ready
./setup.sh

# Build the C++ project
echo "Building C++ engine..."
rm -rf build
mkdir build
cd build
cmake ..
make
cd ..

# Run the application
./run.sh
