#!/bin/bash
mkdir -p build/classes
find stubs -name "*.java" > sources.txt
javac -source 1.8 -target 1.8 -d build/classes @sources.txt
if [ $? -eq 0 ]; then
    jar cf stubs/rt.jar -C build/classes .
    echo "stubs/rt.jar created successfully"
else
    echo "Compilation failed"
fi
