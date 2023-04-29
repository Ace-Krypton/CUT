#include "../include/Reader.hpp"

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

auto Reader::read_data() -> void {
    std::string value;

    const size_t cpus = get_num_cpus();
    _cpu_count_buffer->push(static_cast<int>(cpus));

    while (!_exit_flag.load()) {
        _logger_buffer->push("Reader is reading from /proc/stat");
        std::ifstream file("/proc/stat");

        if (!file) {
            throw std::runtime_error("Failed to open file");
        }
    }
}

auto Reader::start() -> void {
    _thread = std::thread(&Reader::read_data, this);
}

auto Reader::stop() -> void {
    _exit_flag.store(true);
    if (_thread.joinable()) {
        _thread.join();
    }
}
