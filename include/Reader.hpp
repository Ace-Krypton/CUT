#ifndef CUT_READER_HPP
#define CUT_READER_HPP
#pragma once

#include <string>
#include <thread>
#include <fstream>
#include <iostream>

#include "TBuffer.hpp"

class Reader {
public:
    Reader(const std::shared_ptr<TBuffer<std::string>>& logger_buffer,
           const std::shared_ptr<TBuffer<std::string>>& analyzer_buffer,
           const std::shared_ptr<TBuffer<std::size_t>>& cpu_count_buffer)
           : _logger_buffer(logger_buffer),
           _analyzer_buffer(analyzer_buffer),
           _cpu_count_buffer(cpu_count_buffer) { }

    auto stop() -> void;
    auto start() -> void;
    auto read_data() -> void;
    static auto get_num_cpus() -> size_t;

private:
    std::thread _thread;
    std::atomic<bool> _exit_flag { false };
    std::shared_ptr<TBuffer<std::string>> _logger_buffer;
    std::shared_ptr<TBuffer<std::string>> _analyzer_buffer;
    std::shared_ptr<TBuffer<std::size_t>> _cpu_count_buffer;
};

#endif //CUT_READER_HPP
