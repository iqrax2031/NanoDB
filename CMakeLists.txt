# CMakeLists.txt: CMake build definition for NanoDB executables and include paths.
cmake_minimum_required(VERSION 3.10)
project(NanoDB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")  # Enable warnings on Windows

include_directories(${PROJECT_SOURCE_DIR}/include)

# Main NanoDB executable
add_executable(nanodb src/main.cpp src/test_runner.cpp src/pager.cpp src/type.cpp)

# Set output directory
set_target_properties(nanodb PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
