#include "../include/Printer.hpp"

/**
 * @brief Prints the data from the printer buffer.
 */
auto Printer::print_data() -> void {
    /// Continuously analyze data until an exit flag is set
    while(!_exit_flag) {
        /// Push a log message to the logger buffer indicating that data is being printed.
        _logger_buffer->push("Data is being printed");

        /// Wait for data if data stream is empty
        if (_printer_buffer->empty()) {
            std::unique_lock<std::mutex> lock(_mutex);
            _cond_var.wait_for(lock, std::chrono::milliseconds(100));
            continue;
        }

        /// Retrieve the next data string from the data stream
        std::string *cpu_percentage = _printer_buffer->front();

        /// If data string is not empty, then process the data
        if (!cpu_percentage->empty()) {
            /// Print the info to the console by de-referencing it
            std::cout << *cpu_percentage << std::endl;
            /// Update the buffer
            _printer_buffer->pop();
        }
    }
}

/**
 * @brief Starts the Printer thread.
 */
auto Printer::start() -> void {
    _thread = std::thread(&Printer::print_data, this);
}

/**
 * @brief Stops the Printer thread.
 */
auto Printer::stop() -> void {
    _exit_flag.store(true);
    std::unique_lock<std::mutex> lock(_mutex);
    _cond_var.notify_one();
    if (_thread.joinable()) {
        _thread.join();
    }
}
