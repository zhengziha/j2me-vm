#!/bin/bash

# Build script for runtime library (rt.jar)
# Compiles Java source files from stubs/ and creates stubs/rt.jar

echo "Building runtime library (rt.jar)..."

# Create output directory
mkdir -p stubs_build

# Find all Java source files
find stubs -name "*.java" > build/sources.txt

# Compile Java files to stubs_build directory
javac -source 1.8 -target 1.8 -d stubs_build @build/sources.txt

if [ $? -eq 0 ]; then
    # Create JAR file from compiled classes
    jar cf stubs/rt.jar -C stubs_build .
    
    # Clean up
    rm -f build/sources.txt
    rm -rf stubs/rt.jar
    
    echo "stubs/rt.jar created successfully"
    echo "Source files: stubs/"
    echo "Compiled classes: stubs_build/"
else
    echo "Compilation failed"
    rm -f build/sources.txt
fi