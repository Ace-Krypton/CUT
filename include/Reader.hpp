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
    Reader(std::unique_ptr<TBuffer<std::string>> logger_buffer,
           std::unique_ptr<TBuffer<std::string>> analyzer_buffer,
           std::unique_ptr<TBuffer<std::size_t>> cpu_count_buffer)
           : _logger_buffer(std::move(logger_buffer)),
           _analyzer_buffer(std::move(analyzer_buffer)),
           _cpu_count_buffer(std::move(cpu_count_buffer)) { }

    auto start() -> void;
    auto stop() -> void;
    static auto get_num_cpus() -> size_t;
    auto read_data() -> void;

private:
    std::thread _thread;
    std::atomic<bool> _exit_flag { false };
    std::unique_ptr<TBuffer<std::size_t>> _cpu_count_buffer;
    std::unique_ptr<TBuffer<std::string>> _logger_buffer;
    std::unique_ptr<TBuffer<std::string>> _analyzer_buffer;
};

#endif //CUT_READER_HPP
