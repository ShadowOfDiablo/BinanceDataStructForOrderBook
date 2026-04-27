add_library(orderbook
  src/orderbook.cpp
)

target_include_directories(orderbook PUBLIC inc)

if(EXTENDED_DEPTH)
    target_compile_definitions(orderbook PUBLIC EXTENDED_DEPTH)
endif()
