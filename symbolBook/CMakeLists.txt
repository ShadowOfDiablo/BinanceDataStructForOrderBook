add_library(symbolBook
  src/symbolBook.cpp
)

target_include_directories(symbolBook PUBLIC inc)

target_link_libraries(symbolBook PUBLIC orderbook)
