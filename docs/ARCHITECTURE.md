# J2ME 模拟器开发设计文档

## 1. 项目概览
**目标**: 创建一个跨平台的 J2ME (Java 2 Micro Edition) 模拟器，无需安装 JRE 即可运行 JAR 游戏。
**语言**: C++17
**核心功能**:
- **零依赖**: 自包含的 JVM 实现。
- **跨平台**: 支持 Windows, macOS, Linux。
- **格式支持**: 直接执行 `.jar` 文件。

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
- **类加载器 (Class Loader)**: `JarLoader` 和 `ClassParser` 负责解析 `.class` 文件，加载常量池、方法和字段。
- **字节码解释器 (Bytecode Interpreter)**: `Interpreter` 类使用 **Dispatch Table** (分派表) 模式高效执行 JVM 操作码。
- **内存管理器 (Memory Manager)**: `HeapManager` 管理 Java 对象分配。
- **线程管理 (Thread Manager)**: `ThreadManager` 和 `JavaThread` 实现了基于时间片轮转的协作式多线程调度。
- **执行引擎 (Execution Engine)**: 处理方法调用、异常处理和本地方法调用。

### 2.2 平台抽象层 (PAL)
为了确保跨平台兼容性，使用 **SDL2** (Simple DirectMedia Layer)。
- **图形 (Graphics)**: `GraphicsContext` 封装 SDL 渲染器，实现 `javax.microedition.lcdui` 绘图操作。
- **输入 (Input)**: `EventLoop` 将 SDL 键盘事件映射到 J2ME 键码。
- **文件系统 (File System)**: 为 `RMS` (记录管理系统) 抽象文件访问。

### 2.3 运行时库 (CLDC & MIDP)
- **stubs/**: 包含 J2ME 标准库的 Java 接口定义 (如 `java.lang.Object`, `javax.microedition.midlet.MIDlet`)。
- **src/native/**: 包含标准库的本地方法 C++ 实现 (如 `java_lang_System.cpp`)。

## 3. 技术栈

- **编程语言**: C++17
- **构建系统**: CMake
- **外部库**:
  - **SDL2**: 用于窗口、渲染、音频和输入。
  - **Miniz**: 用于解压 `.jar` 文件。

## 4. 目录结构

```
j2me-vm/
├── CMakeLists.txt          # 构建配置
├── README.md               # 项目主文档
├── docs/                   # 详细设计文档
├── src/
│   ├── main.cpp            # 入口点
│   ├── core/               # VM 核心 (Interpreter, HeapManager, ThreadManager)
│   ├── loader/             # JarLoader
│   ├── native/             # 本地方法实现 (NativeRegistry)
│   ├── platform/           # SDL2 封装 (GraphicsContext)
│   └── util/               # 辅助工具
├── stubs/                  # Java 标准库源码
├── stubs_build/            # 编译后的标准库 class 文件
└── tests/                  # 测试用例
```

## 5. 关键技术实现

### 5.1 字节码解释器
采用 **Dispatch Table** 模式，将每个 Opcode 映射到一个 Lambda 函数，替代了传统的巨型 Switch-Case 结构，提高了代码的可维护性和扩展性。

### 5.2 本地方法链接
`NativeRegistry` 维护一个查找表，将 Java 本地方法签名 (如 `java/lang/System.currentTimeMillis()J`) 动态绑定到 C++ 函数。

### 5.3 多线程模型
VM 内部维护自己的线程调度器，不依赖宿主操作系统的线程的一对一映射。`Interpreter::execute` 方法按时间片执行指令，确保 UI 线程 (SDL) 不被阻塞。
