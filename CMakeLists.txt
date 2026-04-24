cmake_minimum_required(VERSION 3.20)
project(dataStructBook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add subdirectories in order of dependency
add_subdirectory(orderBook)
add_subdirectory(symbolBook)

# Main executable
add_executable(symbolbook_main main.cpp)
target_link_libraries(symbolbook_main PRIVATE symbolBook)
