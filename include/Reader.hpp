#pragma once

#include <condition_variable>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "SPSCQueue.hpp"

class Reader {
public:
    Reader(
        const std::shared_ptr<lockfree::SPSCQueue<std::string>>& logger_buffer,
        const std::shared_ptr<lockfree::SPSCQueue<std::string>>&
            analyzer_buffer)
        : _logger_buffer(logger_buffer),
          _analyzer_buffer(analyzer_buffer),
          _exit_flag(false) {}

    auto stop() -> void;
    auto start() -> void;
    auto read_data() -> void;
    auto get_num_cpus() -> std::size_t;

private:
    std::mutex _mutex;
    std::thread _thread;
    std::atomic<bool> _exit_flag;
    std::condition_variable _cond_var;
    std::shared_ptr<lockfree::SPSCQueue<std::string>> _logger_buffer;
    std::shared_ptr<lockfree::SPSCQueue<std::string>> _analyzer_buffer;
};
