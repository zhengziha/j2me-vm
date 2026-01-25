# Windows 构建指南

本指南将帮助您在 Windows 上编译并运行 J2ME-VM 项目。

## 前置条件

### 1. 安装开发工具

1. **安装 MSYS2**
   - 从 [MSYS2 官网](https://www.msys2.org/) 下载并安装 MSYS2
   - 安装后，打开 MSYS2 MinGW 64-bit 终端
   - 运行以下命令更新系统：
     ```bash
     pacman -Syu
     ```
   - 重启终端，再次运行：
     ```bash
     pacman -Syu
     ```

2. **安装必要的开发包**
   - 在 MSYS2 MinGW 64-bit 终端中运行：
     ```bash
     pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make mingw-w64-x86_64-sdl2 mingw-w64-x86_64-sdl2_ttf mingw-w64-x86_64-sdl2_mixer mingw-w64-x86_64-sdl2_image mingw-w64-x86_64-zlib mingw-w64-x86_64-minizip
     ```

3. **安装 Java 开发工具包 (JDK)**
   - 从 [Oracle JDK 官网](https://www.oracle.com/java/technologies/javase-downloads.html) 或 [OpenJDK](https://openjdk.java.net/) 下载并安装 JDK 8 或更高版本
   - 确保 `javac` 和 `jar` 命令在系统 PATH 中可用

### 2. 构建 J2ME 标准库 (rt.jar)

1. **打开 MSYS2 MinGW 64-bit 终端**

2. **导航到项目目录**
   ```bash
   cd /e/devkitPro/examples/j2me-vm
   ```

3. **构建 rt.jar**
   - 运行构建脚本：
     ```bash
     ./build_rt_jar.sh
     ```
   - 或手动执行以下命令：
     ```bash
     mkdir -p build/classes
     find stubs -name "*.java" > build/sources.txt
     javac -source 1.8 -target 1.8 -d build/classes @build/sources.txt
     jar cf stubs/rt.jar -C build/classes .
     ```

### 3. 构建虚拟机

1. **使用 CMake 构建**
   - 在 MSYS2 MinGW 64-bit 终端中运行：
     ```bash
     mkdir -p build
     cd build
     cmake -G "MinGW Makefiles" ..
     make
     ```

2. **或使用 Makefile-windows 构建**
   - 在 MSYS2 MinGW 64-bit 终端中运行：
     ```bash
     make -f Makefile-windows
     ```

### 4. 运行程序

1. **在 MSYS2 MinGW 64-bit 终端中运行**
   ```bash
   ./build/j2me-vm -cp stubs/rt.jar romfs/fr.jar <MainClass>
   ```
   其中 `<MainClass>` 是游戏的主类名，例如 `Main` 或 `GameMIDlet`。

2. **或在 Windows 命令提示符中运行**
   - 确保 `mingw64/bin` 目录在系统 PATH 中
   - 运行：
     ```cmd
     build\j2me-vm.exe -cp stubs\rt.jar romfs\fr.jar <MainClass>
     ```

## 故障排除

### 1. SDL2 相关错误
- 确保已正确安装 SDL2 及其扩展库
- 确保 MSYS2 MinGW 64-bit 终端的 PATH 中包含 SDL2 库路径

### 2. Java 编译错误
- 确保所有 Java 源文件都在正确的目录结构中
- 确保所有依赖的类都被正确编译

### 3. CMake 配置错误
- 确保已正确安装所有必要的依赖项
- 尝试删除 `build` 目录并重新构建

### 4. 运行时错误
- 确保 `stubs/rt.jar` 文件存在且包含所有必要的类
- 确保 `romfs/fr.jar` 文件存在且包含正确的游戏代码
- 检查游戏的主类名是否正确

## 示例

### 运行示例游戏

如果项目中包含示例游戏，例如 `tests/pal.jar`，可以使用以下命令运行：

```bash
./build/j2me-vm -cp stubs/rt.jar:tests/pal.jar pal.GameMIDlet
```

## 注意事项

- 本指南使用 MSYS2 和 MinGW-w64 工具链构建项目，这是在 Windows 上构建 SDL2 应用程序的推荐方法
- 确保在 MSYS2 MinGW 64-bit 终端中执行所有构建命令，而不是在 Windows 命令提示符中
- 如果遇到任何依赖项问题，请参考 MSYS2 文档或相关库的文档

---

如果您按照本指南的步骤操作，应该能够成功在 Windows 上编译并运行 J2ME-VM 项目。如果遇到任何问题，请参考故障排除部分或查看项目的 README.md 文件获取更多信息。