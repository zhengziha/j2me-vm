#!/bin/bash

# Run script for test classes
# Runs compiled test classes from tests/classes

if [ -z "$1" ]; then
    echo "Usage: $0 <ClassName>"
    echo "Example: $0 PrimitiveTypesTest"
    echo ""
    echo "Available test classes:"
    ls tests/src/*.java | sed 's|tests/src/||' | sed 's|.java||'
    exit 1
fi

CLASS_NAME="$1"
CLASS_FILE="tests/classes/${CLASS_NAME}.class"

if [ ! -f "$CLASS_FILE" ]; then
    echo "Error: Class file not found: $CLASS_FILE"
    echo "Please run ./build_tests.sh first."
    exit 1
fi

echo "Running test: $CLASS_NAME"
java -cp tests/classes "$CLASS_NAME"