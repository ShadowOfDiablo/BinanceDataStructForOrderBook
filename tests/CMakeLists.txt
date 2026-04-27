include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(orderbook_tests tests.cpp)
target_link_libraries(orderbook_tests PRIVATE symbolBook GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(orderbook_tests)
