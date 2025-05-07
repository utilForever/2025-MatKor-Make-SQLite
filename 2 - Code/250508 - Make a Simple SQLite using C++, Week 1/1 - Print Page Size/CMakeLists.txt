cmake_minimum_required(VERSION 3.13)

project(simple-sqlite-cpp)

# Enable the C++23 standard
set(CMAKE_CXX_STANDARD 23)

# Set header and source file directories
file(GLOB_RECURSE SOURCE_FILES src/*.hpp src/*.cpp)

# Set executable name
add_executable(sqlite ${SOURCE_FILES})
