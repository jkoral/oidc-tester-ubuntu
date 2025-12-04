#!/bin/bash

# OIDC Tester Launch Script

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Build directory not found. Building application..."
    mkdir -p build
    cd build
    cmake ..
    make
    cd ..
fi

# Check if executable exists
if [ ! -f "build/oidc-tester" ]; then
    echo "Executable not found. Building application..."
    cd build
    make
    cd ..
fi

# Run the application
echo "Launching OIDC Tester..."
./build/oidc-tester

