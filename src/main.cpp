#include <thread>
#include <string>

#include "../include/Reader.hpp"
#include "../include/Analyzer.hpp"

auto main() -> int {
    /// Creating Buffers
    std::shared_ptr<rigtorp::SPSCQueue<std::string>> logger_buffer =
            std::make_unique<rigtorp::SPSCQueue<std::string>>(30);
    std::shared_ptr<rigtorp::SPSCQueue<std::string>> analyzer_buffer =
            std::make_unique<rigtorp::SPSCQueue<std::string>>(30);
    std::shared_ptr<rigtorp::SPSCQueue<std::size_t>> cpu_count_buffer =
            std::make_unique<rigtorp::SPSCQueue<std::size_t>>(30);
    std::shared_ptr<rigtorp::SPSCQueue<int>> printer_buffer =
            std::make_unique<rigtorp::SPSCQueue<int>>(30);

    /// Creating reader thread
    Reader reader(logger_buffer, analyzer_buffer, cpu_count_buffer);

    /// Creating the analyzer thread
    Analyzer analyzer(logger_buffer, analyzer_buffer,
                      printer_buffer, cpu_count_buffer);

    /// Starting and stopping the thread
    reader.start();
    //analyzer.start();
    while (true) {
        std::string *front = analyzer_buffer->front();
        if (front) {
            std::string data = *front;
            if (data.empty()) break;
            std::cout << data << std::endl;
        }
    }
    //analyzer.stop();
    reader.stop();

    return 0;
}
