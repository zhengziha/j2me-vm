Refactoring `Interpreter.cpp` to reduce its size and improve maintainability.

### 1. New Directory Structure
Create a new directory `src/core/instructions/` to house opcode implementations.
- `src/core/instructions/Instructions_Constants.cpp`
- `src/core/instructions/Instructions_Loads.cpp`
- `src/core/instructions/Instructions_Stores.cpp`
- `src/core/instructions/Instructions_Stack.cpp`
- `src/core/instructions/Instructions_Math.cpp`
- `src/core/instructions/Instructions_Conversions.cpp`
- `src/core/instructions/Instructions_Comparisons.cpp`
- `src/core/instructions/Instructions_Control.cpp`
- `src/core/instructions/Instructions_References.cpp`
- `src/core/instructions/Instructions_Extended.cpp`

### 2. Update `Interpreter.hpp`
- Add private helper methods to `Interpreter` class to initialize groups of instructions:
  ```cpp
  void initConstants();
  void initLoads();
  void initStores();
  // ...
  ```
- These methods will be called by `initInstructionTable`.

### 3. Extract Implementation
- Move the lambda implementations from `Interpreter.cpp`'s `initInstructionTable` into the respective new `.cpp` files.
- Each new file will implement one of the `initX` methods (e.g., `void Interpreter::initConstants() { ... }`).
- This keeps the logic within the `Interpreter` class scope (accessing `instructionTable`, `resolveClass`, etc.) but physically separates the code.

### 4. Extract Class Loading Logic
- Move `resolveClass` and its large mocking logic to a new file `src/core/Interpreter_ClassLoader.cpp`.

### 5. Update Build System
- Update `CMakeLists.txt` to include the new `.cpp` files in the build target.

### 6. Verify
- Compile the project to ensure no linking errors.
- Run tests (if available) to ensure functionality remains the same.
