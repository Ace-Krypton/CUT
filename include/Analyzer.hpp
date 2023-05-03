#ifndef CUT_ANALYZER_HPP
#define CUT_ANALYZER_HPP

#include <array>
#include <thread>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "SPSCQueue.hpp"

class Analyzer {
public:
    Analyzer(const std::shared_ptr<rigtorp::SPSCQueue<std::string>>& logger_buffer,
             const std::shared_ptr<rigtorp::SPSCQueue<std::string>>& analyzer_receive,
             const std::shared_ptr<rigtorp::SPSCQueue<int>>& printer_buffer,
             const std::shared_ptr<rigtorp::SPSCQueue<std::size_t>>& cpu_count_receive)
            : _logger_buffer(logger_buffer),
              _analyzer_receive(analyzer_receive),
              _printer_buffer(printer_buffer),
              _cpu_count_receive(cpu_count_receive) { }

    auto stop() -> void;
    auto start() -> void;
    auto analyze_data() -> void;

private:
    std::thread _thread;
    std::mutex _analyzer_mutex;
    std::atomic<bool> _exit_flag { false };
    std::shared_ptr<rigtorp::SPSCQueue<std::string>> _logger_buffer;
    std::shared_ptr<rigtorp::SPSCQueue<int>> _printer_buffer;
    std::shared_ptr<rigtorp::SPSCQueue<std::string>> _analyzer_receive;
    std::shared_ptr<rigtorp::SPSCQueue<std::size_t>> _cpu_count_receive;
};

#endif //CUT_ANALYZER_HPP
