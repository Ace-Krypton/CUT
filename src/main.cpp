#include <thread>
#include <string>
#include <iostream>

#include "../include/TBuffer.hpp"
#include "../include/Reader.hpp"

auto main() -> std::int32_t {
    std::unique_ptr<TBuffer<std::string>> logger_buffer = std::make_unique<TBuffer<std::string>>(10);
    std::unique_ptr<TBuffer<std::string>> analyzer_buffer = std::make_unique<TBuffer<std::string>>(10);
    std::unique_ptr<TBuffer<std::size_t>> cpu_count_buffer = std::make_unique<TBuffer<std::size_t>>(10);

    Reader reader(std::move(logger_buffer),
                  std::move(analyzer_buffer),
                  std::move(cpu_count_buffer));

    reader.start();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    reader.stop();

    std::cout << "Logger buffer:" << std::endl;
    std::unique_ptr<TBuffer<std::string>> logger_buffer_copy = std::move(logger_buffer);
    while (!logger_buffer_copy->empty()) {
        std::cout << logger_buffer_copy->pop() << std::endl;
    }

    std::cout << "Analyzer buffer:" << std::endl;
    std::unique_ptr<TBuffer<std::string>> analyzer_buffer_copy = std::move(analyzer_buffer);
    while (!analyzer_buffer_copy->empty()) {
        std::cout << analyzer_buffer_copy->pop() << std::endl;
    }

    std::cout << "CPU count buffer:" << std::endl;
    std::unique_ptr<TBuffer<std::size_t>> cpu_count_buffer_copy = std::move(cpu_count_buffer);
    while (!cpu_count_buffer_copy->empty()) {
        std::cout << cpu_count_buffer_copy->pop() << std::endl;
    }

    return 0;
}
