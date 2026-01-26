#!/bin/bash

# Build script for test classes
# Compiles Java source files from tests/src to tests/classes

echo "Building test classes..."

# Create classes directory if it doesn't exist
mkdir -p tests/classes

# Compile all Java files
javac -d tests/classes -cp stubs/rt.jar tests/src/*.java

echo "Build completed."
echo "Class files are in: tests/classes/"
echo "Source files are in: tests/src/"