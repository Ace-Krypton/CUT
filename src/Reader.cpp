#include "../include/Reader.hpp"

/**
 * @brief Returns the number of CPUs on the current system.
 *
 * If the '_SC_NPROCESSORS_ONLN' symbol is defined, the function calls sysconf()
 * to get the number of CPUs online. Otherwise, it reads the /proc/stat file to
 * count the number of "cpu" lines.
 *
 * @return The number of CPUs on the current system.
 */
auto Reader::get_num_cpus() -> std::size_t {
#ifdef _SC_NPROCESSORS_ONLN
    /// Use sysconf() to get the number of CPUs online.
    long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cpus > 0) {
        return static_cast<std::size_t>(num_cpus);
    }
#endif

    /// If sysconf() failed or is not available, read /proc/stat.
    std::ifstream count_stream("/proc/stat");

    if (!count_stream.is_open()) {
        /// Inform the logger, if the file could not be opened.
        _logger_buffer->push("Error: Could not open /proc/stat");
        return 0;
    }

    std::string line;
    std::size_t cpu_count = 0;

    while (std::getline(count_stream, line)) {
        /// Count the number of "cpu" lines in the file.
        if (line.find("cpu") == 0) cpu_count++;
    }

    /// Subtract 1 from the count to exclude the "cpu" line that corresponds to all CPUs.
    return cpu_count - 1;
}

/**
 * @brief Reads data from /proc/stat and pushes it into a buffer for analysis.
 */
auto Reader::read_data() -> void {
    /// Continuously read data until the exit flag is set.
    while (!_exit_flag) {
        /// Push a log message to the logger buffer indicating that data is being read.
        _logger_buffer->push("Reader is reading from /proc/stat");

        /// Open the /proc/stat file for reading.
        std::ifstream file("/proc/stat");

        /// If the file cannot be opened, print an error message and wait before continuing.
        if (!file.is_open()) {
            _logger_buffer->push("Error: Could not open /proc/stat");
            std::unique_lock<std::mutex> lock(_mutex);
            _cond_var.wait_for(lock, std::chrono::milliseconds(400));
            continue;
        }

        /// Read the contents of the file into a stringstream.
        std::stringstream ss;
        ss << file.rdbuf();

        std::string line;
        std::size_t count = 0;
        std::stringstream data_ss;

        /// Extract data for each CPU from the file and append it to a stringstream.
        while (std::getline(ss, line)) {
            /// Counts till the number of cores found in CPU
            if (line.find("cpu" + std::to_string(count)) == 0 && count <= get_num_cpus()) {
                data_ss << line << std::endl;
                count++;
            }
        }

        /// Convert the stringstream to a string and push it onto the analyzer buffer for processing.
        std::string raw_data = data_ss.str();
        _analyzer_buffer->push(raw_data);

        /// Close the file and wait for a fixed amount of time before reading again.
        file.close();
        std::unique_lock<std::mutex> lock(_mutex);
        _cond_var.wait_for(lock, std::chrono::seconds(1));
    }
}

/**
 * @brief Starts the reader thread.
 *
 * This method creates a new thread and starts the `read_data` method.
 */
auto Reader::start() -> void {
    _thread = std::thread(&Reader::read_data, this);
}

/**
 * @brief Stops the reader thread.
 *
 * This method sets the `_exit_flag` variable to `true`, notifies the condition variable
 * to wake up the thread, and waits for the thread to join before returning.
 */
auto Reader::stop() -> void {
    _exit_flag.store(true);
    std::unique_lock<std::mutex> lock(_mutex);
    _cond_var.notify_one();
    if (_thread.joinable()) {
        _thread.join();
    }
}
