# J2ME-VM 编译配置文档

本文档详细介绍了J2ME-VM项目在不同操作系统下的编译脚本、编译目录和编译输出。

---

## 目录

- [Windows 系统](#windows-系统)
- [Linux/macOS 系统](#linuxmacos-系统)
- [Nintendo Switch 系统](#nintendo-switch-系统)
- [通用构建配置](#通用构建配置)
- [测试相关脚本](#测试相关脚本)
- [Java运行时库构建](#java运行时库构建)
- [目录结构对比](#目录结构对比)
- [依赖库对比](#依赖库对比)

---

## Windows 系统

### 编译脚本

- **`Makefile-windows`** - 主编译脚本
- **`build_rt_jar.ps1`** - Java运行时库构建脚本

### 编译命令

```bash
make -f Makefile-windows
```

### 编译目录

- `build/` - 临时编译文件目录
- `build/classes/` - Java类文件编译目录
- `build/sources.txt` - Java源文件列表

### 编译输出

- `j2me-vm.exe` - 主可执行文件（约5.8MB）
- `stubs/rt.jar` - Java运行时库

### 依赖库

- SDL2, SDL2_mixer, SDL2_image, SDL2_ttf
- zlib, minizip
- 编译器：MinGW-w64 GCC

### 编译器配置

```makefile
CXX = D:\msys64\mingw64\bin\g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./src -IE:/sdl2 -ID:\msys64\mingw64\include
LDFLAGS = -LE:/sdl2/lib -LD:\msys64\mingw64\lib
LIBS = -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_ttf -lz -lminizip -mconsole
```

---

## Linux/macOS 系统

### 编译脚本

- **`Makefile`** - CMake生成的Makefile
- **`build.sh`** - 快速构建脚本
- **`build_rt_jar.sh`** - Java运行时库构建脚本

### 编译命令

```bash
./build.sh
# 或
mkdir -p build && cd build && cmake .. && make
```

### 编译目录

- `build/` - CMake构建目录
- `build/classes/` - Java类文件编译目录

### 编译输出

- `j2me-vm` - 主可执行文件
- `stubs/rt.jar` - Java运行时库

### 依赖库

- SDL2, SDL2_mixer, SDL2_image, SDL2_ttf
- zlib, minizip
- 编译器：系统GCC或Clang

### 快速构建脚本内容

```bash
#!/bin/bash
mkdir -p build && cd build && cmake .. && make
```

---

## Nintendo Switch 系统

### 编译脚本

- **`Makefile-ns`** - Switch专用编译脚本

### 编译命令

```bash
make -f Makefile-ns
```

### 编译目录

- `build/` - 临时编译文件目录

### 编译输出

- `j2me-vm.nro` - Switch homebrew可执行文件（默认）
- `j2me-vm.nsp` - Switch系统模块可执行文件（如果有config.json）
- `j2me-vm.nacp` - 应用元数据文件

### 依赖库

- devkitPro开发环境
- libnx
- SDL2 (Switch版本)
- 编译器：devkitPro ARM工具链

### 特殊配置

- 需要设置 `DEVKITPRO` 环境变量
- 目标架构：ARMv8-A (Cortex-A57)
- 支持 RomFS 文件系统

### 编译器配置

```makefile
ARCH := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
CFLAGS := `$(PREFIX)pkg-config --cflags sdl2 SDL2_mixer SDL2_image SDL2_ttf` -Wall -O2 -ffunction-sections $(ARCH) $(DEFINES)
CFLAGS += $(INCLUDE) -D__SWITCH__ -I$(DEVKITPRO)/portlibs/switch/include/minizip
CXXFLAGS := $(CFLAGS) -D__SWITCH__
LDFLAGS = -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)
LIBS := `$(PREFIX)pkg-config --libs sdl2 SDL2_mixer SDL2_image SDL2_ttf` -lz -lminizip -lnx
```

---

## 通用构建配置

### CMake配置

- **`CMakeLists.txt`** - 跨平台CMake配置文件
- **`cmake_install.cmake`** - 安装配置

### CMake构建命令

```bash
mkdir -p build
cd build
cmake ..
make
```

### CMake生成器选项

- Windows: `cmake -G "MinGW Makefiles" ..`
- Linux/macOS: `cmake ..` (默认Unix Makefiles)
- Visual Studio: `cmake -G "Visual Studio 16 2019" ..`

---

## 测试相关脚本

### 测试脚本

- **`run_test.sh`** - 运行测试脚本
- **`build_tests.sh`** - 构建测试脚本
- **`build_image_test.sh`** - 图像测试构建脚本

### 测试输出

- 测试可执行文件（位于build目录）

---

## Java运行时库构建

### Windows版本

```powershell
.\build_rt_jar.ps1
```

### Linux/macOS版本

```bash
./build_rt_jar.sh
```

### 编译过程

1. 扫描 `stubs/` 目录下的所有Java文件
2. 编译Java文件到临时目录
3. 打包生成 `stubs/rt.jar`

### Windows PowerShell脚本要点

```powershell
# 生成sources.txt
$javaFiles = Get-ChildItem -Path "stubs" -Recurse -Filter "*.java"
Clear-Content -Path "build/sources.txt" -ErrorAction SilentlyContinue
foreach ($file in $javaFiles) {
    $relativePath = $file.FullName -replace [regex]::Escape($PWD.Path), ''
    $relativePath = $relativePath -replace '^\\', ''
    Add-Content -Path "build/sources.txt" -Value $relativePath
}

# 编译Java文件
javac -source 1.8 -target 1.8 -d "build/classes" @build/sources.txt

# 创建JAR文件
jar cf "stubs/rt.jar" -C "build/classes" .
```

### Linux/macOS Shell脚本要点

```bash
# 创建输出目录
mkdir -p stubs_build

# 查找所有Java源文件
find stubs -name "*.java" > build/sources.txt

# 编译Java文件到stubs_build目录
javac -source 1.8 -target 1.8 -d stubs_build @build/sources.txt

# 创建JAR文件
jar cf stubs/rt.jar -C stubs_build .
```

---

## 目录结构对比

| 目录/文件 | Windows | Linux/macOS | Nintendo Switch |
|-----------|---------|-------------|-----------------|
| 主编译脚本 | Makefile-windows | Makefile (CMake) | Makefile-ns |
| 快速构建脚本 | - | build.sh | - |
| Java构建脚本 | build_rt_jar.ps1 | build_rt_jar.sh | - |
| 编译目录 | build/ | build/ | build/ |
| 可执行文件 | j2me-vm.exe | j2me-vm | j2me-vm.nro/.nsp |
| 运行时库 | stubs/rt.jar | stubs/rt.jar | stubs/rt.jar |
| 编译器 | MinGW-w64 GCC | GCC/Clang | devkitPro ARM GCC |

---

## 依赖库对比

| 库名称 | Windows | Linux/macOS | Nintendo Switch |
|--------|---------|-------------|-----------------|
| SDL2 | ✅ | ✅ | ✅ (Switch版本) |
| SDL2_mixer | ✅ | ✅ | ✅ |
| SDL2_image | ✅ | ✅ | ✅ |
| SDL2_ttf | ✅ | ✅ | ✅ |
| zlib | ✅ | ✅ | ✅ |
| minizip | ✅ | ✅ | ✅ |
| libnx | ❌ | ❌ | ✅ |

---

## 项目目录结构

### 根目录文件

```
j2me-vm/
├── CMakeLists.txt              # CMake配置文件
├── Makefile                    # Linux/macOS Makefile (CMake生成)
├── Makefile-windows            # Windows Makefile
├── Makefile-ns                 # Nintendo Switch Makefile
├── build.sh                    # Linux/macOS快速构建脚本
├── build_rt_jar.sh             # Linux/macOS Java库构建脚本
├── build_rt_jar.ps1            # Windows Java库构建脚本
├── build_tests.sh              # 测试构建脚本
├── build_image_test.sh         # 图像测试构建脚本
├── run.sh                      # Linux/macOS运行脚本
├── run_test.sh                 # 测试运行脚本
├── README.md                   # 项目说明文档
├── WINDOWS_BUILD.md            # Windows构建说明
└── docs/                       # 文档目录
```

### 源代码目录

```
src/
├── main.cpp                    # 主程序入口
├── core/                       # 核心功能模块
│   ├── EventLoop.cpp/hpp       # 事件循环
│   ├── J2MEVM.cpp/hpp          # 虚拟机主类
│   ├── Interpreter.cpp/hpp     # 字节码解释器
│   ├── ClassParser.cpp/hpp     # 类文件解析器
│   ├── HeapManager.cpp/hpp      # 堆内存管理器
│   ├── NativeRegistry.cpp/hpp   # 本地方法注册表
│   ├── Diagnostics.cpp/hpp      # 诊断工具
│   ├── Logger.cpp/hpp           # 日志系统
│   ├── StackFrame.cpp/hpp       # 栈帧管理
│   ├── RuntimeTypes.cpp/hpp     # 运行时类型
│   ├── Interpreter_ClassLoader.cpp  # 类加载器
│   └── instructions/            # 字节码指令实现
│       ├── Instructions_Comparisons.cpp
│       ├── Instructions_Constants.cpp
│       ├── Instructions_Control.cpp
│       ├── Instructions_Conversions.cpp
│       ├── Instructions_Extended.cpp
│       ├── Instructions_Loads.cpp
│       ├── Instructions_Math.cpp
│       ├── Instructions_References.cpp
│       ├── Instructions_Stack.cpp
│       └── Instructions_Stores.cpp
├── loader/                     # 类加载器模块
│   └── JarLoader.cpp/hpp        # JAR文件加载器
├── native/                      # 本地方法实现
│   ├── java_lang_String.cpp/hpp
│   ├── java_lang_System.cpp/hpp
│   ├── java_lang_Thread.cpp/hpp
│   ├── java_lang_Class.cpp/hpp
│   ├── java_lang_StringBuffer.cpp/hpp
│   ├── java_lang_Object.cpp/hpp
│   ├── java_lang_Float.cpp/hpp
│   ├── java_lang_Double.cpp/hpp
│   ├── java_lang_Math.cpp/hpp
│   ├── java_io_PrintStream.cpp/hpp
│   ├── java_io_InputStream.cpp/hpp
│   ├── java_util_Timer.cpp/hpp
│   ├── NativeInputStream.cpp/hpp
│   ├── ImageCommon.cpp/hpp
│   ├── javax_microedition_lcdui_Graphics.cpp/hpp
│   ├── javax_microedition_lcdui_Display.cpp/hpp
│   ├── javax_microedition_lcdui_Image.cpp/hpp
│   ├── javax_microedition_lcdui_Font.cpp/hpp
│   ├── javax_microedition_lcdui_game_GameCanvas.cpp/hpp
│   ├── javax_microedition_lcdui_game_Sprite.cpp/hpp
│   ├── javax_microedition_lcdui_game_TiledLayer.cpp/hpp
│   ├── javax_microedition_media_Manager.cpp/hpp
│   ├── javax_microedition_rms_RecordStore.cpp/hpp
│   └── javax_microedition_io_Connector.cpp/hpp
├── platform/                   # 平台相关代码
│   └── GraphicsContext.cpp/hpp  # 图形上下文
└── util/                       # 工具类
    ├── DataReader.hpp
    └── FileUtils.cpp/hpp
```

### Java存根目录

```
stubs/
├── META-INF/
│   └── MANIFEST.MF
├── com/nokia/mid/ui/           # Nokia特定扩展
├── j2me/media/                 # J2ME媒体扩展
├── java/                       # Java标准库存根
│   ├── io/
│   ├── lang/
│   └── util/
├── javax/microedition/         # J2ME标准库存根
│   ├── io/
│   ├── lcdui/
│   │   └── game/              # 游戏API
│   ├── media/
│   ├── midlet/
│   └── rms/
└── rt.jar                      # 编译后的运行时库
```

### 构建输出目录

```
build/                          # 通用构建目录
├── classes/                    # Java类文件
├── sources.txt                 # Java源文件列表
└── [其他CMake生成的文件]
```

### 文档目录

```
docs/
├── ARCHITECTURE.md             # 架构文档
├── J2ME规范.md                 # J2ME规范文档
├── Phase 2 Implementation Plan.md
├── Phase 4 Implementation Plan.md
├── Refactoring Interpreter.cpp.md
├── 测试结果报告.md
├── 测试计划.md
├── 项目完成度评估.md
└── BUILD_CONFIG.md             # 本文档
```

### 资源目录

```
fonts/                          # 字体文件
├── Tahoma.ttf
└── s60snr.ttf
```

---

## 构建流程总结

### Windows构建流程

1. **准备环境**
   - 安装MSYS2和MinGW-w64
   - 安装SDL2相关库
   - 配置环境变量

2. **构建Java运行时库**
   ```powershell
   .\build_rt_jar.ps1
   ```

3. **编译主程序**
   ```bash
   make -f Makefile-windows
   ```

4. **运行程序**
   ```bash
   .\j2me-vm.exe
   ```

### Linux/macOS构建流程

1. **准备环境**
   - 安装SDL2相关库
   - 确保GCC或Clang可用

2. **构建Java运行时库**
   ```bash
   ./build_rt_jar.sh
   ```

3. **编译主程序**
   ```bash
   ./build.sh
   ```

4. **运行程序**
   ```bash
   ./j2me-vm
   ```

### Nintendo Switch构建流程

1. **准备环境**
   - 安装devkitPro
   - 设置DEVKITPRO环境变量

2. **构建Java运行时库**
   ```bash
   ./build_rt_jar.sh
   ```

3. **编译主程序**
   ```bash
   make -f Makefile-ns
   ```

4. **部署到Switch**
   - 将生成的 `j2me-vm.nro` 复制到Switch的SD卡

---

## 常见问题

### Windows构建问题

**问题1：找不到libfreetype-6.dll**
- 解决方案：安装mingw-w64-x86_64-freetype包

**问题2：undefined reference to WinMain**
- 解决方案：在Makefile-windows中添加-mconsole链接选项

**问题3：文字显示为方框**
- 解决方案：添加系统字体支持（如SimSun.ttc）

### Linux/macOS构建问题

**问题1：找不到SDL2库**
- 解决方案：安装SDL2开发包
  - Ubuntu/Debian: `sudo apt-get install libsdl2-dev`
  - macOS: `brew install sdl2`

**问题2：CMake版本过低**
- 解决方案：升级CMake到3.10或更高版本

### Nintendo Switch构建问题

**问题1：DEVKITPRO环境变量未设置**
- 解决方案：在~/.bashrc中添加 `export DEVKITPRO=/path/to/devkitpro`

**问题2：找不到libnx**
- 解决方案：确保devkitPro完整安装

---

## 贡献指南

如果您想为项目添加新的平台支持或改进现有构建系统，请遵循以下步骤：

1. 创建对应平台的Makefile（如Makefile-xxx）
2. 添加必要的构建脚本
3. 更新本文档
4. 测试构建流程
5. 提交Pull Request

---

## 许可证

请参考项目根目录的LICENSE文件。

---

## 联系方式

如有构建相关问题，请通过以下方式联系：

- 提交Issue到项目仓库
- 查看项目文档
- 参考现有构建脚本

---

*最后更新：2026-01-27*