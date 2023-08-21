#pragma once

#include <ncurses.h>

#include <condition_variable>
#include <iostream>
#include <string>
#include <thread>

#include "SPSCQueue.hpp"

class Printer {
   public:
    Printer(
        const std::shared_ptr<lockfree::SPSCQueue<std::string>>& logger_buffer,
        const std::shared_ptr<lockfree::SPSCQueue<std::string>>& printer_buffer)
        : _logger_buffer(logger_buffer),
          _printer_buffer(printer_buffer),
          _exit_flag(false) {}

    auto stop() -> void;
    auto start() -> void;
    auto print_data() -> void;

    [[maybe_unused]] auto draw() -> void;
    [[maybe_unused]] static auto progress_bar(const std::string& percent)
        -> std::string;

   private:
    std::mutex _mutex;
    std::thread _thread;
    std::atomic<bool> _exit_flag;
    std::condition_variable _cond_var;
    std::shared_ptr<lockfree::SPSCQueue<std::string>> _logger_buffer;
    std::shared_ptr<lockfree::SPSCQueue<std::string>> _printer_buffer;
};
