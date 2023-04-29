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

    const std::size_t cpus = get_num_cpus();
    _cpu_count_buffer->push(static_cast<int>(cpus));

    for (int a = 0; a < 5; ++a) {
        std::cout << "Inside while loop" << std::endl;
        _logger_buffer->push("Reader is reading from /proc/stat");
        std::ifstream file("/proc/stat");

        if (!file.is_open()) {
            std::cerr << "Error: Could not open /proc/stat" << std::endl;
            std::this_thread::sleep_for(std::chrono::nanoseconds(200000));
            continue;
        }

        std::getline(file, value);
        std::cout << "Read value: " << value << std::endl;

        for (std::size_t i = 0; i < cpus; ++i) {
            std::getline(file, value);
            _analyzer_buffer->push(value);
        }
        std::cout << "TILL HERE" << std::endl;

        file.close();
        std::this_thread::sleep_for(std::chrono::nanoseconds(200000));
    }

    _analyzer_buffer->push("Reader Finished");
    std::cout << "READER FINISHED" << std::endl;
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
