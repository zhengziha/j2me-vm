cmake_minimum_required(VERSION 3.10)
project(j2me-vm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use local SDL2 installation
set(SDL2_INCLUDE_DIRS "E:/sdl2/SDL2")
set(SDL2_LIBRARIES "E:/sdl2/lib/libSDL2.dll.a")
set(SDL2_TTF_LIB "E:/sdl2/lib/libSDL2_ttf.dll.a")
set(SDL2_MIXER_LIB "E:/sdl2/lib/libSDL2_mixer.dll.a")
set(SDL2_IMAGE_LIB "E:/sdl2/lib/libSDL2_image.dll.a")

# Verify SDL2 installation
if(NOT EXISTS "${SDL2_INCLUDE_DIRS}/SDL.h")
    message(FATAL_ERROR "SDL2 header not found at ${SDL2_INCLUDE_DIRS}/SDL.h")
endif()
if(NOT EXISTS "${SDL2_LIBRARIES}")
    message(FATAL_ERROR "SDL2 library not found at ${SDL2_LIBRARIES}")
endif()
if(NOT EXISTS "${SDL2_TTF_LIB}")
    message(FATAL_ERROR "SDL2_ttf library not found at ${SDL2_TTF_LIB}")
endif()
if(NOT EXISTS "${SDL2_MIXER_LIB}")
    message(FATAL_ERROR "SDL2_mixer library not found at ${SDL2_MIXER_LIB}")
endif()
if(NOT EXISTS "${SDL2_IMAGE_LIB}")
    message(FATAL_ERROR "SDL2_image library not found at ${SDL2_IMAGE_LIB}")
endif()

# Find minizip (optional for Windows)
find_library(MINIZIP_LIB minizip)
if(NOT MINIZIP_LIB AND NOT WIN32)
    message(FATAL_ERROR "minizip not found!")
endif()

# Find zlib (optional for Windows)
find_package(ZLIB)
if(NOT ZLIB_FOUND AND NOT WIN32)
    message(FATAL_ERROR "ZLIB not found!")
endif()

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    /usr/local/include/SDL2
    /opt/homebrew/include/SDL2
    /mingw64/include
    /mingw64/include/SDL2
    "E:/sdl2/SDL2"
)

# Source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

# Executable
add_executable(j2me-vm ${SOURCES})

# Link libraries
target_link_libraries(j2me-vm 
    ${SDL2_LIBRARIES} 
    ${SDL2_TTF_LIB} 
    ${SDL2_MIXER_LIB} 
    ${SDL2_IMAGE_LIB} 
)

# Link zlib if found
if(ZLIB_FOUND)
    target_link_libraries(j2me-vm ${ZLIB_LIBRARIES})
endif()

# Link minizip if found
if(MINIZIP_LIB)
    target_link_libraries(j2me-vm ${MINIZIP_LIB})
endif()
