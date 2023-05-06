#pragma once

#include <thread>
#include <string>
#include <iostream>
#include <condition_variable>

#include "SPSCQueue.hpp"

class Printer {
public:
    Printer(const std::shared_ptr<lockfree::SPSCQueue<int>>& printer_buffer,
            const std::shared_ptr<lockfree::SPSCQueue<std::string>>& logger_buffer)
            : _logger_buffer(logger_buffer),
              _printer_buffer(printer_buffer),
              _exit_flag(false) { }

    auto stop() -> void;
    auto start() -> void;
    auto print_data() -> void;

private:
    std::mutex _mutex;
    std::thread _thread;
    std::atomic<bool> _exit_flag;
    std::condition_variable _cond_var;
    std::shared_ptr<lockfree::SPSCQueue<int>> _printer_buffer;
    std::shared_ptr<lockfree::SPSCQueue<std::string>> _logger_buffer;
};
