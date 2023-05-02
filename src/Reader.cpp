#include "../include/Reader.hpp"

/**
 * @brief Returns the number of available CPUs on the system.
 * @return The number of available CPUs as a size_t value.
 */
auto Reader::get_num_cpus() -> size_t {
#ifdef _SC_NPROCESSORS_ONLN
    long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cpus > 0) {
        return static_cast<size_t>(num_cpus);
    }
#endif

    std::ifstream count_stream("/proc/stat");

    if (!count_stream.is_open()) {
        std::cerr << "Error: Could not open /proc/stat" << std::endl;
        return 0;
    }

    std::string line;
    size_t cpu_count = 0;

    while (std::getline(count_stream, line)) {
        if (line.find("cpu") == 0) cpu_count++;
    }

    return cpu_count - 1;
}

/**
 * @brief Reads data from "/proc/stat" and pushes it onto the analyzer buffer.
 */
auto Reader::read_data() -> void {
    const size_t cpus = get_num_cpus();
    _cpu_count_buffer->push(static_cast<int>(cpus));

    while (!_exit_flag) {
        _logger_buffer->push("Reader is reading from /proc/stat");
        std::ifstream file("/proc/stat");

        if (!file.is_open()) {
            std::cerr << "Error: Could not open /proc/stat" << std::endl;
            std::this_thread::sleep_for(std::chrono::nanoseconds(200000));
            continue;
        }

        std::stringstream ss;
        ss << file.rdbuf();

        std::string line;
        std::stringstream data_ss;
        while (std::getline(ss, line)) {
            if (line.find("cpu0") == 0 ||
                line.find("cpu1") == 0 ||
                line.find("cpu2") == 0 ||
                line.find("cpu3") == 0) {
                data_ss << line << std::endl;
            }
        }

        std::string raw_data = data_ss.str();
        _analyzer_buffer->push(raw_data);

        file.close();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    _analyzer_buffer->push("Reader Finished");
}

/**
 * @brief Starts the Reader thread.
 */
auto Reader::start() -> void {
    std::cout << "STARTING" << std::endl;
    _thread = std::thread(&Reader::read_data, this);
}

/**
 * @brief Stops the Reader thread.
 */
auto Reader::stop() -> void {
    std::cout << "CAME TO HERE" << std::endl;
    _exit_flag.store(true);
    if (_thread.joinable()) {
        _thread.join();
    }
}
