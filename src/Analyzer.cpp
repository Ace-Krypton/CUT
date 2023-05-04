#include <map>
#include "../include/Analyzer.hpp"

struct CPUData {
    unsigned long long int user;
    unsigned long long int nice;
    unsigned long long int system;
    unsigned long long int idle;
    unsigned long long int iowait;
    unsigned long long int irq;
    unsigned long long int softirq;
    unsigned long long int steal;
    unsigned long long int guest;
    unsigned long long int guest_nice;
};

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
            std::cout << *raw_data << std::endl;
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
                    std::uint64_t total_time = 0;
                    std::uint64_t idle_time = times[3];
                    for (std::size_t i = 0; i < times.size(); ++i) total_time += times[i] - prev_times[i];
                    std::uint64_t cpu_usage = 100 * (total_time - idle_time) / total_time;
                    std::cout << "CPU " << cpu_name << " usage: " << cpu_usage << "%" << std::endl;
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

auto Analyzer::analyze_data() -> void {
    const std::size_t cpus = *_cpu_count_receive->front();
    _cpu_count_receive->pop();
    auto* cpus_current_data = new CPUData[cpus];
    auto* cpus_previous_data = new CPUData[cpus];
    std::string data;

    for (std::size_t i = 0; i < cpus; ++i) {
        data = *_analyzer_receive->front();
        _analyzer_receive->pop();
        std::istringstream iss(data);
        std::string cpu_name;
        iss >> cpu_name >> cpus_previous_data[i].user >> cpus_previous_data[i].nice >> cpus_previous_data[i].system
            >> cpus_previous_data[i].idle >> cpus_previous_data[i].iowait >> cpus_previous_data[i].irq
            >> cpus_previous_data[i].softirq >> cpus_previous_data[i].steal >> cpus_previous_data[i].guest
            >> cpus_previous_data[i].guest_nice;
    }

    while (!_exit_flag) {
        bool data_received = false;
        for (std::size_t i = 0; i < cpus; ++i) {
            data = *_analyzer_receive->front();
            _analyzer_receive->pop();
            data_received = true;
            std::istringstream iss(data);
            std::string cpu_name;
            iss >> cpu_name >> cpus_current_data[i].user >> cpus_current_data[i].nice >> cpus_current_data[i].system
                >> cpus_current_data[i].idle >> cpus_current_data[i].iowait >> cpus_current_data[i].irq
                >> cpus_current_data[i].softirq >> cpus_current_data[i].steal >> cpus_current_data[i].guest
                >> cpus_current_data[i].guest_nice;
        }

        if (!data_received) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        for (std::size_t i = 0; i < cpus; ++i) {
            unsigned long long int prev_idle = cpus_previous_data[i].idle + cpus_previous_data[i].iowait;
            unsigned long long int idle = cpus_current_data[i].idle + cpus_current_data[i].iowait;
            unsigned long long int prev_non_idle = cpus_previous_data[i].user + cpus_previous_data[i].nice
                                                   + cpus_previous_data[i].system + cpus_previous_data[i].irq
                                                   + cpus_previous_data[i].softirq + cpus_previous_data[i].steal;
            unsigned long long int non_idle = cpus_current_data[i].user + cpus_current_data[i].nice
                                              + cpus_current_data[i].system + cpus_current_data[i].irq
                                              + cpus_current_data[i].softirq + cpus_current_data[i].steal;
            unsigned long long int prev_total = prev_idle + prev_non_idle;
            unsigned long long int total = idle + non_idle;
            unsigned long long int total_d = total - prev_total;
            unsigned long long int idle_d  = idle - prev_idle;

            int cpu_usage = (total_d == 0) ? 0 : (int) (100 * (total_d - idle_d) / total_d);
            std::cout << "cpu: " << "- " << cpu_usage << "%" << std::endl;
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
