# Tests Directory Structure

## Overview
This directory contains test files for the J2ME VM, organized into separate directories for source files and compiled classes.

## Directory Structure

```
tests/
├── src/              # Java source files (.java)
├── classes/          # Compiled class files (.class)
├── graphics/         # Graphics-related test resources and JARs
├── resources/        # Resource test JARs
├── rms/            # RMS (RecordStore) test JARs
├── *.jar           # Pre-built test JAR files
├── build_tests.sh   # Script to compile test classes
└── run_test.sh      # Script to run test classes
```

## Building Test Classes

To compile all test source files:

```bash
./build_tests.sh
```

This will compile all `.java` files from `tests/src/` to `tests/classes/`.

## Running Test Classes

To run a specific test class:

```bash
./run_test.sh <ClassName>
```

For example:
```bash
./run_test.sh PrimitiveTypesTest
```

To see all available test classes:
```bash
./run_test.sh
```

## Running Pre-built JAR Tests

To run pre-built JAR test files:

```bash
./j2me-vm tests/<jar_file>
```

For example:
```bash
./j2me-vm tests/PrimitiveTypesTest.jar
./j2me-vm tests/pal.jar
```

## Available Test Classes

- `ArrayTest` - Array operations
- `BytecodeTest` - Bytecode interpretation
- `CollectionTest` - Collections framework (Vector, Hashtable, Stack)
- `DebugStringTest` - String debugging
- `ExceptionTest` - Exception handling
- `GraphicsTest` - Graphics operations
- `IOTest` - I/O operations
- `ImageTest` - Image handling
- `MainTest` - Main method execution
- `MathTest` - Math operations
- `ObjectToStringTest` - Object.toString() functionality
- `PrimitiveTypesTest` - Primitive type operations
- `RMSTest` - RecordStore operations
- `ResourceTest` - Resource loading
- `SimpleStringTest` - Basic string operations
- `StringTest` - String operations
- `StreamTest` - Stream operations
- `TestImageMIDlet` - Image MIDlet
- `ThreadTest` - Thread operations

## Notes

- Source files (.java) are in `tests/src/`
- Compiled class files (.class) are in `tests/classes/`
- Pre-built JAR files remain in the root `tests/` directory
- Resource files (images, etc.) remain in their respective subdirectories
- The `build_tests.sh` script compiles all source files
- The `run_test.sh` script runs compiled test classes