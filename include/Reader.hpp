#ifndef CUT_READER_HPP
#define CUT_READER_HPP

#include <string>
#include <vector>
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>
#include <condition_variable>

#include "SPSCQueue.hpp"

class Reader {
public:
    Reader(const std::shared_ptr<rigtorp::SPSCQueue<std::string>>& logger_buffer,
           const std::shared_ptr<rigtorp::SPSCQueue<std::string>>& analyzer_buffer,
           const std::shared_ptr<rigtorp::SPSCQueue<std::size_t>>& cpu_count_buffer)
            : _logger_buffer(logger_buffer),
              _analyzer_buffer(analyzer_buffer),
              _cpu_count_buffer(cpu_count_buffer),
              _exit_flag(false) { }

    auto stop() -> void;
    auto start() -> void;
    auto read_data() -> void;
    static auto get_num_cpus() -> std::size_t;

public:
    std::mutex _mutex;
    std::thread _thread;
    std::condition_variable _cond_var;
    std::atomic<bool> _exit_flag { false };
    std::shared_ptr<rigtorp::SPSCQueue<std::string>> _logger_buffer;
    std::shared_ptr<rigtorp::SPSCQueue<std::string>> _analyzer_buffer;
    std::shared_ptr<rigtorp::SPSCQueue<std::size_t>> _cpu_count_buffer;
};

#endif //CUT_READER_HPP
