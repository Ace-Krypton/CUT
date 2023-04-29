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
    std::string value;

    const size_t cpus = get_num_cpus();
    _cpu_count_buffer->push(static_cast<int>(cpus));

    for (int a = 0; a < 5; ++a) {
        _logger_buffer->push("Reader is reading from /proc/stat");
        std::ifstream file("/proc/stat");

        if (!file.is_open()) {
            std::cerr << "Error: Could not open /proc/stat" << std::endl;
            std::this_thread::sleep_for(std::chrono::nanoseconds(200000));
            continue;
        }

        std::getline(file, value);

        for (std::size_t i = 0; i < cpus; ++i) {
            std::getline(file, value);
            _analyzer_buffer->push(value);
        }

        file.close();
        std::this_thread::sleep_for(std::chrono::nanoseconds(200000));
    }

    _analyzer_buffer->push("Reader Finished");
}

/**
 * @brief Starts the Reader thread.
 */
auto Reader::start() -> void {
    _thread = std::thread(&Reader::read_data, this);
}

/**
 * @brief Stops the Reader thread.
 */
auto Reader::stop() -> void {
    _exit_flag.store(true);
    if (_thread.joinable()) {
        _thread.join();
    }
}
