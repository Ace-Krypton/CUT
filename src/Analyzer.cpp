#include <map>
#include "../include/Analyzer.hpp"

auto Analyzer::analyze() -> void {
    std::map<std::string, std::vector<std::uint64_t>> prev_cpu_data;

    while (!_exit_flag) {
        if (_analyzer_receive->empty()) {
            std::unique_lock<std::mutex> lock(_mutex);
            _cond_var.wait_for(lock, std::chrono::milliseconds(100));
            continue;
        }

        std::string *raw_data = _analyzer_receive->front();
        if (raw_data) {
            std::stringstream ss(*raw_data);
            std::string line;
            std::map<std::string, std::vector<std::uint64_t>> cpu_data;

            while (std::getline(ss, line)) {
                std::istringstream iss(line);
                std::string cpu_name;
                iss >> cpu_name;
                if (cpu_name == "cpu") continue;
                std::vector<std::uint64_t> times;
                std::uint64_t time;
                while (iss >> time) times.push_back(time);
                cpu_data[cpu_name] = times;
            }

            if (!prev_cpu_data.empty()) {
                for (const auto& [cpu_name, times] : cpu_data) {
                    const auto& prev_times = prev_cpu_data[cpu_name];
                    if (prev_times.size() != times.size()) continue;
                    double total_time = 0;
                    double idle_time = 0;
                    for (std::size_t i = 0; i < times.size(); ++i) {
                        total_time += static_cast<double>(times[i] - prev_times[i]);
                        if (i == 3) idle_time = static_cast<double>(times[i] - prev_times[i]);
                    }
                    if (total_time > 0) {
                        double cpu_usage = 100.0 * (total_time - idle_time) / total_time;
                        std::cout << "CPU " << cpu_name << " usage: " << cpu_usage << "%" << std::endl;
                    }
                }
            }

            prev_cpu_data = cpu_data;
            _analyzer_receive->pop();
        }

        std::size_t *cpu_data = _cpu_count_receive->front();
        if (cpu_data) {
            _cpu_count_receive->pop();
        }

        std::string *logger_data = _logger_buffer->front();
        if (logger_data) {
            _logger_buffer->pop();
        }
    }
}

auto Analyzer::start() -> void {
    _thread = std::thread(&Analyzer::analyze, this);
}

auto Analyzer::stop() -> void {
    _exit_flag.store(true);
    std::unique_lock<std::mutex> lock(_mutex);
    _cond_var.notify_one();
    if (_thread.joinable()) {
        _thread.join();
    }
}
