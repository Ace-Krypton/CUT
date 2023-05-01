#include "../include/Analyzer.hpp"

struct CPUData {
    std::string cpu;

    unsigned long long int user { };
    unsigned long long int nice { };
    unsigned long long int system { };
    unsigned long long int idle { };
    unsigned long long int iowait { };
    unsigned long long int irq { };
    unsigned long long int softirq { };
    unsigned long long int steal { };
    unsigned long long int guest { };
    unsigned long long int guest_nice { };

    [[nodiscard]] unsigned long long int get_total() const {
        return user + nice + system + idle + iowait + irq + softirq + steal;
    }
};

auto Analyzer::analyze_data() -> void {
    while (!_exit_flag) {
        std::unique_ptr<std::string> raw_data;

        {
            std::lock_guard<std::mutex> lock(_analyzer_mutex);
            raw_data = _analyzer_receive->pop();
        }

        std::stringstream ss(*raw_data);

        std::vector<CPUData> data;
        std::string line;

        while (std::getline(ss, line)) {
            if (line.empty() || line.find("cpu") != 0) {
                continue;
            }

            std::istringstream iss(line);
            CPUData cpu_data;
            iss >> cpu_data.cpu >> cpu_data.user >> cpu_data.nice >> cpu_data.system >> cpu_data.idle
                >> cpu_data.iowait >> cpu_data.irq >> cpu_data.softirq >> cpu_data.steal
                >> cpu_data.guest >> cpu_data.guest_nice;
            data.push_back(cpu_data);
        }

        for (auto it = std::next(data.begin()); it != data.end(); ++it) {
            double utilization = 100.0 *
                                 (1.0 - (static_cast<double>(it->idle) / static_cast<double>(it->get_total())));

            std::stringstream _ss;
            _ss << std::fixed << std::setprecision(1) << utilization;
            std::string utilization_str = _ss.str();
            std::cout << it->cpu + " utilization: " + utilization_str + "%" << std::endl;
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
