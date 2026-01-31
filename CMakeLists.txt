cmake_minimum_required(VERSION 3.10)
project(j2me-vm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 REQUIRED)

# Find SDL2_ttf
find_library(SDL2_TTF_LIB SDL2_ttf)
if(NOT SDL2_TTF_LIB)
    message(FATAL_ERROR "SDL2_ttf not found!")
endif()

# Find SDL2_mixer
find_library(SDL2_MIXER_LIB SDL2_mixer)
if(NOT SDL2_MIXER_LIB)
    message(FATAL_ERROR "SDL2_mixer not found!")
endif()

# Find libzip
find_package(libzip REQUIRED)

# Find minizip
find_library(MINIZIP_LIB minizip)
if(NOT MINIZIP_LIB)
    message(FATAL_ERROR "minizip not found! Run: brew install minizip")
endif()

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
    /usr/local/include/SDL2
    /opt/homebrew/include/SDL2
    /opt/homebrew/include
)

# Source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

# Executable
add_executable(j2me-vm ${SOURCES})

# Link libraries
target_link_libraries(j2me-vm ${SDL2_LIBRARIES} ${SDL2_TTF_LIB} ${SDL2_MIXER_LIB} ${MINIZIP_LIB} libzip::zip iconv)
