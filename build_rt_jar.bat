@echo off
REM Build script for runtime library (rt.jar) on Windows
REM Compiles Java source files from stubs and creates stubs/rt.jar

echo Building runtime library (rt.jar)...

REM Create output directory
if not exist stubs_build mkdir stubs_build

REM Find all Java source files
dir /s /b stubs\*.java > build\sources.txt

REM Compile Java files to stubs_build directory
javac -encoding UTF-8 -source 1.8 -target 1.8 -d stubs_build @build\sources.txt

if %errorlevel% equ 0 (
    REM Remove old rt.jar
    if exist stubs\rt.jar del stubs\rt.jar
    
    REM Create JAR file from compiled classes
    jar cf stubs\rt.jar -C stubs_build .
    
    REM Clean up
    del build\sources.txt
    
    echo stubs\rt.jar created successfully
    echo Source files: stubs/
    echo Compiled classes: stubs_build/
) else (
    echo Compilation failed
    del build\sources.txt
)
