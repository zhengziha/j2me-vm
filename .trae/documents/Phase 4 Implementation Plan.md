# Phase 4 & Optimization Implementation Plan

## Current Status
We have a working Phase 3 implementation with:
- Native method registry and binding.
- SDL2 GraphicsContext wrapper.
- Basic graphics operations (`setColor`, `fillRect`, `drawLine`).
- Input handling infrastructure in `main.cpp` (event loop capturing keys).
- InputTest.java verifying key events are dispatched to `keyPressed`.

However, the current `InputTest` is running in a headless environment (trae-sandbox), so we can't visually verify the interaction, but we can verify via console logs. The previous run showed "Class not found" errors because `GraphicsTest` or `InputTest` were trying to look up classes that might not be fully loaded or registered in our simple map if not in the JAR or stubs were not included in the runtime JAR properly. Wait, `test_v6.jar` includes `InputTest.class` but maybe not `javax/microedition/lcdui/Canvas.class`?
Actually, the `test_v6.jar` creation command `javac ... -cp stubs ...` compiles against stubs, but we need to ensure `Canvas.class` and `Graphics.class` are also *inside* the JAR or available to the VM. In the last command, we only put `InputTest.class` in `test_v6.jar`. We should include the `javax` classes too.

## Goal
Refine the runtime environment to support full interaction simulation and resource loading.

### 1. Fix Class Loading for Runtime
- [ ] Ensure `javax.microedition.lcdui.*` classes (from stubs) are included in the runtime JAR or loaded from a "system classpath".
- [ ] Verify `InputTest` runs without "Class not found" errors.

### 2. Implement `Resource` Loading
- [ ] Implement `Class.getResourceAsStream(String name)`.
- [ ] Create a `NativeInputStream` or similar to read from JAR/File.
- [ ] Add `javax.microedition.lcdui.Image` class stub and native `createImage`.

### 3. Enhance Graphics
- [ ] Implement `drawImage`.
- [ ] Improve `drawString` (maybe a simple bitmap font loader if time permits, or just better debug output).

### 4. Verification
- [ ] Create a `ResourceTest` that loads a text file or image from JAR.
