# J2ME 模拟器开发设计文档

## 1. 项目概览
**目标**: 创建一个跨平台的 J2ME (Java 2 Micro Edition) 模拟器，无需安装 JRE 即可运行 JAR 游戏。
**语言**: C/C++ (推荐使用 C++17 以获得现代内存管理和抽象特性)。
**核心功能**:
- **零依赖**: 自包含的 JVM 实现。
- **跨平台**: 支持 Windows, macOS, Linux, 以及潜在的 Android/WebAssembly (通过 Emscripten)。
- **格式支持**: 直接执行 `.jar` 和 `.jad` 文件。

## 2. 架构设计

模拟器分为四个主要层级：

```
+-------------------------------------------------------+
|                   J2ME 应用程序 (JAR)                  |
+-------------------------------------------------------+
|             MIDP / CLDC Java 类库                     |
|    (java.lang.*, javax.microedition.*, java.io.*)     |
+-------------------------------------------------------+
|                   本地接口 (KNI/JNI)                   |
+-------------------------------------------------------+
|                    虚拟机核心 (VM Core)                |
|  [加载器] [解释器] [内存管理] [线程管理]                 |
+-------------------------------------------------------+
|               平台抽象层 (PAL)                         |
|      (SDL2 负责图形、音频、输入、文件系统)               |
+-------------------------------------------------------+
```

### 2.1 虚拟机核心 (The Engine)
- **类加载器 (Class Loader)**: 解析 `.class` 文件，验证格式 (魔数 `0xCAFEBABE`)，并加载常量池、方法和字段。
- **字节码解释器 (Bytecode Interpreter)**: 处理 JVM 操作码的取指-解码-执行循环。
- **内存管理器 (Memory Manager)**:
  - **堆 (Heap)**: 管理 Java 对象。实现垃圾回收器 (简单的标记-清除算法即可)。
  - **方法区 (Method Area)**: 存储已加载的类和静态变量。
  - **栈 (Stack)**: 每个线程的执行栈 (帧、局部变量、操作数栈)。
- **执行引擎 (Execution Engine)**: 处理方法调用、异常处理和本地方法调用。

### 2.2 平台抽象层 (PAL)
为了确保跨平台兼容性，我们将使用 **SDL2** (Simple DirectMedia Layer)。
- **图形 (Graphics)**: 将 MIDP `Canvas` 和 `Image` 缓冲区渲染到 SDL 纹理。
- **音频 (Audio)**: 使用 SDL_Mixer 播放 PCM 数据或 MIDI 文件。
- **输入 (Input)**: 将键盘/触摸事件映射到 J2ME 键码 (例如 `KEY_NUM5`, `KEY_SOFT1`)。
- **文件系统 (File System)**: 为 `RMS` (记录管理系统) 抽象文件访问。

### 2.3 运行时库 (CLDC & MIDP)
我们需要实现标准库的本地方法。
- **CLDC (Connected Limited Device Configuration)**: 核心 Java 类 (`Object`, `String`, `Thread`, `System`)。
- **MIDP (Mobile Information Device Profile)**: UI 和游戏特定类 (`Canvas`, `Graphics`, `Sprite`, `RecordStore`)。

## 3. 技术栈

- **编程语言**: C++17
- **构建系统**: CMake (C++ 跨平台项目的标准)。
- **外部库**:
  - **SDL2**: 用于窗口、渲染、音频和输入。
  - **Miniz** 或 **Zlib**: 用于解压 `.jar` 文件。
  - **TinyXML2** (可选): 如果需要解析 `.jad` 描述文件。

## 4. 目录结构

```
j2me-vm/
├── CMakeLists.txt          # 构建配置
├── docs/                   # 文档
├── src/
│   ├── main.cpp            # 入口点
│   ├── core/               # VM 核心
│   │   ├── interpreter.cpp # 字节码执行
│   │   ├── classloader.cpp # .class 解析
│   │   ├── memory.cpp      # GC 和对象分配
│   │   └── thread.cpp      # 线程支持
│   ├── loader/             # JAR/ZIP 处理
│   ├── native/             # 本地方法实现 (javax_microedition_lcdui_Graphics.cpp)
│   ├── platform/           # SDL2 封装
│   └── util/               # 辅助函数
├── lib/                    # 第三方库 (SDL2, miniz)
└── classes/                # Java 标准库实现 (.java 文件，需编译为 .class)
```

## 5. 开发路线图

### 第一阶段：基础建设 (Phase 1)
1.  **项目设置**: 初始化 CMake 并链接 SDL2。
2.  **JAR 加载**: 实现 ZIP 解压以读取 `MANIFEST.MF` 和 `.class` 文件。
3.  **类解析**: 解析类文件的常量池、字段、方法和属性。
4.  **基础解释器**: 实现核心操作码 (算术、栈操作) 和简单的 "Hello World" 输出。

### 第二阶段：面向对象执行 (Phase 2)
1.  **内存模型**: 实现 `JavaObject` 和 `JavaClass` 结构。
2.  **方法调用**: 实现 `invokevirtual`, `invokespecial`, `invokestatic`。
3.  **继承与接口**: 处理方法解析。

### 第三阶段：图形与交互 (Phase 3)
1.  **UI 框架**: 实现 `javax.microedition.lcdui.Canvas`。
2.  **渲染**: 将 J2ME 绘图命令 (`drawLine`, `drawImage`) 映射到 SDL2 原语。
3.  **输入处理**: 将键盘事件映射到 J2ME 键码。

### 第四阶段：音频与持久化 (Phase 4)
1.  **RMS**: 使用本地文件实现记录管理系统。
2.  **音频**: 实现 `javax.microedition.media` 以进行基础声音播放。

## 6. 关键技术挑战与解决方案

### 6.1 字节序 (Endianness)
Java .class 文件使用 **大端序 (Big-Endian)**。大多数现代 CPU 是小端序。
*解决方案*: 创建一个 `DataReader` 工具类，在读取 `u2` 和 `u4` 类型时自动交换字节。

### 6.2 本地方法链接 (Native Method Linking)
J2ME 严重依赖本地代码进行图形和 IO 操作。
*解决方案*: 维护一个查找表，将 Java 方法签名 (例如 `com/nokia/mid/ui/DirectUtils.getDirectGraphics()`) 映射到 C++ 函数指针。

### 6.3 垃圾回收 (Garbage Collection)
*解决方案*: 从简单的 "Stop-the-World" 标记-清除收集器开始。当堆分配失败时触发它。

## 7. 构建指南

### 前置条件
- C++ 编译器 (GCC, Clang, 或 MSVC)
- CMake 3.10+
- SDL2 开发库

### 构建步骤
```bash
mkdir build && cd build
cmake ..
make
./j2me-vm path/to/game.jar
```
