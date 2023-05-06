#include "../include/Analyzer.hpp"

/**
 * @brief Continuously analyzes CPU data received by the analyzer thread.
 *
 * This function reads CPU data from the analyzer's receive buffer, processes it, and prints the CPU usage
 * for each CPU to the console. It runs continuously until the `_exit_flag` member is set to true.
 */
auto Analyzer::analyze_data() -> void {
    /// Map to store previous CPU data
    std::map<std::string, std::vector<std::uint64_t>> prev_cpu_data;

    /// Continuously analyze data until an exit flag is set
    while (!_exit_flag) {
        /// Wait for data if data stream is empty
        if (_analyzer_receive->empty()) {
            std::unique_lock<std::mutex> lock(_mutex);
            _cond_var.wait_for(lock, std::chrono::milliseconds(100));
            continue;
        }

        /// Retrieve the next data string from the data stream
        std::string *raw_data = _analyzer_receive->front();

        /// If data string is not empty, then process the data
        if (!raw_data->empty()) {
            /// Parse the data string to obtain the current CPU data
            std::stringstream ss(*raw_data);
            std::string line;
            std::map<std::string, std::vector<std::uint64_t>> cpu_data;
            while (std::getline(ss, line)) {
                std::istringstream stream(line);
                std::string cpu_name;
                stream >> cpu_name;
                if (cpu_name == "cpu") continue;
                std::vector<std::uint64_t> times;
                std::uint64_t time;
                while (stream >> time) times.push_back(time);
                cpu_data[cpu_name] = times;
            }

            /// Calculate CPU usage based on the current and previous CPU data
            if (!prev_cpu_data.empty()) {
                for (const auto& [cpu_name, times] : cpu_data) {
                    const std::vector<std::uint64_t> &prev_times = prev_cpu_data[cpu_name];
                    if (prev_times.size() != times.size()) continue;
                    double total_time = 0;
                    double idle_time = 0;
                    for (std::size_t i = 0; i < times.size(); ++i) {
                        total_time += static_cast<double>(times[i] - prev_times[i]);
                        if (i == 3) idle_time = static_cast<double>(times[i] - prev_times[i]);
                    } if (total_time > 0) {
                        double cpu_usage_floating = 100.0 * (total_time - idle_time) / total_time;
                        int cpu_usage_int = static_cast<int>(cpu_usage_floating);
                        _printer_buffer->push(cpu_name + " usage: " + std::to_string(cpu_usage_int) + "%");
                    }
                }
            }

            /// Update previous CPU data to current CPU data
            prev_cpu_data = cpu_data;
            /// Update the Buffer by removing last element
            _analyzer_receive->pop();
        }

        std::string *logger_data = _logger_buffer->front();
        if (logger_data) {
            _logger_buffer->pop();
        }
    }
}

/**
 * @brief Starts the Analyzer thread.
 *
 * This function creates a new thread that continuously analyzes CPU data received by the Analyzer thread
 * and prints the CPU usage for each CPU to the console.
 */
auto Analyzer::start() -> void {
    _thread = std::thread(&Analyzer::analyze_data, this);
}

/**
 * @brief Stops the Analyzer thread.
 *
 * This function sets the `_exit_flag` member variable to true, notifies the Analyzer thread to wake up, and
 * waits for the thread to join.
 */
auto Analyzer::stop() -> void {
    _exit_flag.store(true);
    std::unique_lock<std::mutex> lock(_mutex);
    _cond_var.notify_one();
    if (_thread.joinable()) {
        _thread.join();
    }
}
