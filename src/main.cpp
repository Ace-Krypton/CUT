#include <thread>
#include <string>

#include "../include/Reader.hpp"
#include "../include/Analyzer.hpp"

auto main() -> int {
    /// Creating Buffers
    std::shared_ptr<TBuffer<std::string>> logger_buffer = std::make_unique<TBuffer<std::string>>(20);
    std::shared_ptr<TBuffer<std::string>> analyzer_buffer = std::make_unique<TBuffer<std::string>>(30);
    std::shared_ptr<TBuffer<std::size_t>> cpu_count_buffer = std::make_unique<TBuffer<std::size_t>>(1);
    std::shared_ptr<TBuffer<int>> printer_buffer = std::make_unique<TBuffer<int>>(30);

    /// Creating reader thread
    Reader reader(logger_buffer, analyzer_buffer, cpu_count_buffer);

    /// Creating the analyzer thread
    Analyzer analyzer(logger_buffer, analyzer_buffer,
                      printer_buffer, cpu_count_buffer);

    /// Starting and stopping the thread
    reader.start();
    //std::this_thread::sleep_for(std::chrono::milliseconds (200));
    //analyzer.start();
    while (true) {
        std::unique_ptr<std::string> data = analyzer_buffer->pop();
        if (*data == "Reader Finished") break;
        std::cout << *data << std::endl;
    }
    reader.stop();
    //analyzer.stop();

    /// Printing the printer buffer

    /// Printing the analyzer buffer
    /*while (!(analyzer_buffer->empty())) {
        std::cout << *analyzer_buffer->pop() << std::endl;
    }*/

    /// Printing the logger buffer
    /*while (!(logger_buffer->empty())) {
        std::cout << *logger_buffer->pop();
    }*/

    /// Printing the cpu count buffer
    /*while (!(cpu_count_buffer->empty())) {
        std::cout << *cpu_count_buffer->pop();
    }*/

    return 0;
}
