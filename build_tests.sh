#!/bin/bash

# Build script for test classes
# Compiles Java source files from tests/src and creates tests.jar

echo "Building test classes..."

# Create classes directory if it doesn't exist
mkdir -p tests/classes

# Compile all Java files
javac -d tests/classes -cp stubs/rt.jar tests/src/*.java

if [ $? -eq 0 ]; then
    # Create JAR file from compiled classes
    jar cfm tests.jar tests/META-INF/MANIFEST.MF -C tests/classes .
    
    echo "tests.jar created successfully"
    echo "Source files: tests/src/"
    echo "Compiled classes: tests/classes/"
else
    echo "Compilation failed"
fi