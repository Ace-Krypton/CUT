#include <thread>
#include <string>

#include "../include/Reader.hpp"
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

auto analyze_data(const std::shared_ptr<lockfree::SPSCQueue<std::string>> &analyzer_buffer) -> void {
    while (true) {
        std::string *analyze = analyzer_buffer->front();
        analyzer_buffer->pop();
        auto* cpus_current_data = new CPUData[4];
        auto* cpus_previous_data = new CPUData[4];

        if (analyze) {
            std::string data = *analyze;
            if (data.empty()) break;

            for (std::size_t i = 0; i < 4; ++i) {
                //data = *_analyzer_receive->front();
                std::istringstream iss(data);
                std::string cpu_name;
                iss >> cpu_name >> cpus_previous_data[i].user >> cpus_previous_data[i].nice >> cpus_previous_data[i].system
                    >> cpus_previous_data[i].idle >> cpus_previous_data[i].iowait >> cpus_previous_data[i].irq
                    >> cpus_previous_data[i].softirq >> cpus_previous_data[i].steal >> cpus_previous_data[i].guest
                    >> cpus_previous_data[i].guest_nice;
            }
        }
    }
}


auto main() -> int {
    /// Creating Buffers
    std::shared_ptr<lockfree::SPSCQueue<std::string>> logger_buffer =
            std::make_unique<lockfree::SPSCQueue<std::string>>(20);
    std::shared_ptr<lockfree::SPSCQueue<std::string>> analyzer_buffer =
            std::make_unique<lockfree::SPSCQueue<std::string>>(30);
    std::shared_ptr<lockfree::SPSCQueue<std::size_t>> cpu_count_buffer =
            std::make_unique<lockfree::SPSCQueue<std::size_t>>(1);
    std::shared_ptr<lockfree::SPSCQueue<int>> printer_buffer =
            std::make_unique<lockfree::SPSCQueue<int>>(30);

    /// Creating reader thread
    Reader reader(logger_buffer, analyzer_buffer, cpu_count_buffer);

    /// Creating the analyzer thread
    Analyzer analyzer(logger_buffer, analyzer_buffer,
                      printer_buffer, cpu_count_buffer);

    /// Starting and stopping the thread
    //std::thread analyzer_thread(analyze_data, analyzer_buffer);
    reader.start();
    while (true) {
        // THIS CODE SHOULD ANALYZE THE CPU PERCENTAGE ACCORDING TO THE DATA THAT COMES FROM THE READER
        std::string *analyze = analyzer_buffer->front();
        if (analyze) {
            std::string data = *analyze;
            if (data.empty()) break;
            std::cout << data << std::endl;
            analyzer_buffer->pop();
        }
    }
    reader.stop();
    //analyzer_thread.join();

    return 0;
}
