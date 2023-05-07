cmake_minimum_required(VERSION 3.25)
project(CUT)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/master.zip
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-lncursesw")
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-interference-size")
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        CUT
        src/main.cpp
        src/TBuffer.cpp
        src/Reader.cpp
        include/Reader.hpp
        src/Analyzer.cpp
        include/Analyzer.hpp
        include/SPSCQueue.hpp
        src/Printer.cpp
        include/Printer.hpp
        src/Logger.cpp
        include/Logger.hpp
        tests/ReaderTest.cpp
        tests/ReaderTest.hpp
)

target_link_libraries(
        CUT
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(CUT)
