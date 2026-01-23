# J2ME Emulator Implementation Plan

## Current Status
We have successfully completed Phase 1 of the development:
- **Project Structure**: CMake build system with SDL2 and LibZip dependencies.
- **JarLoader**: Can read manifest and class files from JARs.
- **ClassParser**: Can parse Java `.class` files (Constant Pool, Methods, Fields, Attributes).
- **Basic Interpreter**: Can execute a simple "Hello World" program by mocking `System.out.println`.

## Next Steps: Phase 2 - Object Oriented Execution

The goal of Phase 2 is to move from a simple "Hello World" hack to a proper Object-Oriented VM.

### 1. Memory Model & Object System
- [ ] **Design `JavaObject` and `JavaClass` Runtime Structures**:
    - `JavaClass`: Runtime representation of a parsed class (linking constant pools, method tables).
    - `JavaObject`: Heap instance with a pointer to `JavaClass` and field storage.
- [ ] **Implement Heap Manager**:
    - Simple bump-pointer or free-list allocator.
    - Basic Garbage Collection (Mark-and-Sweep) foundation.

### 2. Enhanced Interpreter
- [ ] **Implement `new` Opcode**: Allocate objects on the heap.
- [ ] **Implement `putfield` / `getfield`**: Access instance variables.
- [ ] **Implement `invokespecial`**: For calling constructors (`<init>`).
- [ ] **Stack Frame Improvements**: Support `this` pointer in local variable 0 for instance methods.

### 3. Class Loading & Linking
- [ ] **Recursive Class Loading**: When `new Foo()` is encountered, load `Foo.class` if not loaded.
- [ ] **Hierarchy Resolution**: Handle superclasses.

### 4. Verification
- [ ] Create a test case with a custom class (e.g., `Point` class with `x`, `y` fields).
- [ ] Verify object creation and field access.
