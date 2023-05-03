#include <vector>
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

auto Analyzer::analyze_data() -> void {
    const std::size_t cpus = *_cpu_count_receive->front();
    auto* cpus_current_data = new CPUData[cpus];
    auto* cpus_previous_data = new CPUData[cpus];
    std::string data;

    for (std::size_t i = 0; i < cpus; ++i) {
        data = *_analyzer_receive->front();
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
    _thread = std::thread(&Analyzer::analyze_data, this);
}

auto Analyzer::stop() -> void {
    _exit_flag.store(true);
    if (_thread.joinable()) {
        _thread.join();
    }
}
