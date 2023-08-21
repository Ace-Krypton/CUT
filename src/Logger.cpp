#include "../include/Logger.hpp"

/**
 * @brief Logger receives messages from all threads and writes them into a file.
 */
auto Logger::logger_thread() -> void {
    /// Opens the file in appended mode
    std::ofstream log_file(_file_name, std::ios::app);

    /// Continuously analyze data until an exit flag is set
    while (!_exit_flag) {
        /// Wait for data if data stream is empty
        if (_logger_buffer->empty()) {
            std::unique_lock<std::mutex> lock(_mutex);
            _cond_var.wait_for(lock, std::chrono::milliseconds(100));
            continue;
        }

        /// Retrieve the next data string from the data stream
        std::string *logs = _logger_buffer->front();

        /// If data string is not empty, then process the data
        if (!logs->empty()) {
            /// Write the logs to the file
            log_file << *logs << std::endl;
            _logger_buffer->pop();
        }
    }
}

/**
 * @brief Starts the Printer thread.
 *
 * This method creates a new thread and starts the `logger_thread` method.
 */
auto Logger::start() -> void {
    _thread = std::thread(&Logger::logger_thread, this);
}

/**
 * @brief Stops the Printer thread.
 *
 * This method sets the `_exit_flag` variable to `true`, notifies the condition
 * variable to wake up the thread, and waits for the thread to join before
 * returning.
 */
auto Logger::stop() -> void {
    _exit_flag.store(true);
    std::unique_lock<std::mutex> lock(_mutex);
    _cond_var.notify_one();
    if (_thread.joinable()) {
        _thread.join();
    }
}
