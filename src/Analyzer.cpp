#include "../include/Analyzer.hpp"

auto Analyzer::analyze_data() -> void {
    std::string value;

    const size_t cpus = *_cpu_count_receive->pop();

    std::vector<long long> prev_idle_times(cpus, 0);
    std::vector<long long> prev_total_times(cpus, 0);

    while (true) {
        value = *_analyzer_receive->pop();

        if (value == "Reader Finished") {
            _printer_buffer->push("FINISHED");
            break;
        }

        std::stringstream ss(value);
        std::string cpu_name;
        long long user_time, nice_time, system_time,
        idle_time, io_wait_time, irq_time, soft_irq_time;

        ss >> cpu_name >> user_time >> nice_time >> system_time
        >> idle_time >> io_wait_time >> irq_time >> soft_irq_time;

        if (cpu_name.find("cpu") == 0) {
            size_t cpu_index = std::stoi(cpu_name.substr(3));
            long long total_time = user_time + nice_time + system_time
                    + idle_time + io_wait_time + irq_time + soft_irq_time;

            long long idle_time_diff = idle_time - prev_idle_times[cpu_index];
            long long total_time_diff = total_time - prev_total_times[cpu_index];
            double idle_percentage =
                    static_cast<double>(idle_time_diff) / static_cast<double>(total_time_diff);
            double usage_percentage = (1 - idle_percentage) * 100;

            prev_idle_times[cpu_index] = idle_time;
            prev_total_times[cpu_index] = total_time;

            std::stringstream stream;
            stream << std::fixed << std::setprecision(0) << usage_percentage;
            std::string usage_str = stream.str();
            _printer_buffer->push(usage_str);
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
