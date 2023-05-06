#include <thread>
#include <string>

#include "../include/Reader.hpp"
#include "../include/Printer.hpp"
#include "../include/Analyzer.hpp"

auto main() -> int {
    /// Creating Buffers
    std::shared_ptr<lockfree::SPSCQueue<std::string>> logger_buffer =
            std::make_unique<lockfree::SPSCQueue<std::string>>(20);
    std::shared_ptr<lockfree::SPSCQueue<std::string>> analyzer_buffer =
            std::make_unique<lockfree::SPSCQueue<std::string>>(30);
    std::shared_ptr<lockfree::SPSCQueue<std::string>> printer_buffer =
            std::make_unique<lockfree::SPSCQueue<std::string>>(30);

    /// Creating reader thread
    Reader reader(logger_buffer, analyzer_buffer);

    /// Creating the analyzer thread
    Analyzer analyzer(logger_buffer, printer_buffer,
                      analyzer_buffer);

    /// Creating the printer thread
    Printer printer(logger_buffer, printer_buffer);

    /// Starting the threads
    reader.start();
    analyzer.start();
    printer.start();

    /// I will fix this
    std::this_thread::sleep_for(std::chrono::seconds(1000));

    /// Stopping the threads
    reader.stop();
    analyzer.stop();
    printer.stop();

    return 0;
}
