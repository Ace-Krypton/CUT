#pragma once

#include <condition_variable>
#include <fstream>
#include <thread>
#include <utility>

#include "SPSCQueue.hpp"

class Logger {
   public:
    Logger(
        const std::shared_ptr<lockfree::SPSCQueue<std::string>>& logger_buffer,
        std::string file_name)
        : _logger_buffer(logger_buffer),
          _file_name(std::move(file_name)),
          _exit_flag(false) {}

    auto stop() -> void;
    auto start() -> void;
    auto logger_thread() -> void;

   private:
    std::mutex _mutex;
    std::thread _thread;
    std::string _file_name;
    std::atomic<bool> _exit_flag;
    std::condition_variable _cond_var;
    std::shared_ptr<lockfree::SPSCQueue<std::string>> _logger_buffer;
};
