#include "../include/Printer.hpp"

auto Printer::print_data() -> void {
    while(!_exit_flag) {
        if (_printer_buffer->empty()) {
            std::unique_lock<std::mutex> lock(_mutex);
            _cond_var.wait_for(lock, std::chrono::milliseconds(100));
            continue;
        }

        int *cpu_percentage = _printer_buffer->front();

        if (cpu_percentage != nullptr) {
            std::cout << *cpu_percentage << "%" << std::endl;
            _printer_buffer->pop();
        }
    }
}

auto Printer::start() -> void {
    _thread = std::thread(&Printer::print_data, this);
}

auto Printer::stop() -> void {
    _exit_flag.store(true);
    std::unique_lock<std::mutex> lock(_mutex);
    _cond_var.notify_one();
    if (_thread.joinable()) {
        _thread.join();
    }
}
