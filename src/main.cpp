#include <thread>
#include <string>

#include "../include/Reader.hpp"
#include "../include/Analyzer.hpp"

auto main() -> int {
    /// Creating Buffers
    std::shared_ptr<lockfree::SPSCQueue<std::string>> logger_buffer =
            std::make_unique<lockfree::SPSCQueue<std::string>>(20);
    std::shared_ptr<lockfree::SPSCQueue<std::string>> analyzer_buffer =
            std::make_unique<lockfree::SPSCQueue<std::string>>(30);
    std::shared_ptr<lockfree::SPSCQueue<std::size_t>> cpu_count_buffer =
            std::make_unique<lockfree::SPSCQueue<std::size_t>>(1);
    std::shared_ptr<lockfree::SPSCQueue<int>> printer_buffer =
            std::make_unique<lockfree::SPSCQueue<int>>(30);

    /// Creating reader thread
    Reader reader(logger_buffer, analyzer_buffer, cpu_count_buffer);

    /// Creating the analyzer thread
    Analyzer analyzer(logger_buffer, analyzer_buffer,
                      printer_buffer, cpu_count_buffer);

    /// Starting the threads
    reader.start();
    analyzer.start();
    std::this_thread::sleep_for(std::chrono::seconds(50));
    reader.stop();
    analyzer.stop();

    return 0;
}
