cmake_minimum_required(VERSION 3.20)
project(PricePointBook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library for the order book logic
add_library(orderbook
  orderBook/src/orderbook.cpp
)

target_include_directories(orderbook PUBLIC orderBook/inc)

# Main executable
add_executable(symbolbook_main main.cpp)
target_link_libraries(symbolbook_main PRIVATE orderbook)
