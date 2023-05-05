#pragma once

#include <map>
#include <array>
#include <mutex>
#include <thread>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <condition_variable>

#include "SPSCQueue.hpp"

class Analyzer {
public:
    Analyzer(const std::shared_ptr<lockfree::SPSCQueue<int>>& printer_buffer,
             const std::shared_ptr<lockfree::SPSCQueue<std::string>>& logger_buffer,
             const std::shared_ptr<lockfree::SPSCQueue<std::string>>& analyzer_receive,
             const std::shared_ptr<lockfree::SPSCQueue<std::size_t>>& cpu_count_receive)
            : _printer_buffer(printer_buffer),
              _logger_buffer(logger_buffer),
              _analyzer_receive(analyzer_receive),
              _cpu_count_receive(cpu_count_receive),
              _exit_flag(false) { }

    auto stop() -> void;
    auto start() -> void;
    auto analyze_data() -> void;

private:
    std::mutex _mutex;
    std::thread _thread;
    std::atomic<bool> _exit_flag;
    std::condition_variable _cond_var;
    std::shared_ptr<lockfree::SPSCQueue<int>> _printer_buffer;
    std::shared_ptr<lockfree::SPSCQueue<std::string>> _logger_buffer;
    std::shared_ptr<lockfree::SPSCQueue<std::string>> _analyzer_receive;
    std::shared_ptr<lockfree::SPSCQueue<std::size_t>> _cpu_count_receive;
};
