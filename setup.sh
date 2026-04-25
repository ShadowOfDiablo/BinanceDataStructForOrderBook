#!/bin/bash
set -e

echo "--- Initializing Project Environment ---"

# 1. Check for required system tools
for tool in cmake make g++ node npm curl; do
    if ! command -v $tool &> /dev/null; then
        echo "ERROR: Required tool '$tool' is not installed."
        exit 1
    fi
done
echo "System tools verified."

# 2. Download nlohmann/json (C++ JSON Library)
JSON_INC_DIR="orderBook/inc"
JSON_HPP="$JSON_INC_DIR/json.hpp"

if [ ! -f "$JSON_HPP" ]; then
    echo "Installing C++ JSON library to $JSON_INC_DIR..."
    mkdir -p "$JSON_INC_DIR"
    curl -L https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp -o "$JSON_HPP"
else
    echo "C++ JSON library already present."
fi

# 3. Install Node.js dependencies (WebSocket Client)
if [ -f "package.json" ]; then
    echo "Installing Node.js dependencies..."
    npm install
else
    echo "WARNING: package.json not found, skipping npm install."
fi

# 4. Ensure scripts are executable
chmod +x build.sh run.sh
echo "Permissions updated."

echo "--- Setup Complete ---"
