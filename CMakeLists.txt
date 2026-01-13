cmake_minimum_required(VERSION 3.10)
project(j2me-vm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 REQUIRED)

# Find libzip
find_package(libzip REQUIRED)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Executable
add_executable(j2me-vm ${SOURCES})

# Link libraries
target_link_libraries(j2me-vm ${SDL2_LIBRARIES} libzip::zip)
