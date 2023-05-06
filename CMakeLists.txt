cmake_minimum_required(VERSION 3.25)
project(CUT)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-interference-size")

add_executable(CUT src/main.cpp src/TBuffer.cpp src/Reader.cpp include/Reader.hpp
        src/Analyzer.cpp include/Analyzer.hpp include/SPSCQueue.hpp src/Printer.cpp include/Printer.hpp)