#include <thread>
#include <string>
#include <csignal>
#include <benchmark/benchmark.h>

#include "../include/Reader.hpp"
#include "../include/Logger.hpp"
#include "../include/Printer.hpp"
#include "../include/Analyzer.hpp"

std::atomic<bool> terminate(false);

auto signal_handler(int signal) -> void {
    terminate = true;
    (void)(signal);
    std::exit(0);
}

static auto BM_Main(benchmark::State& state) -> void {
    while (state.KeepRunning()) {
        /// Creating Buffers
        std::shared_ptr<lockfree::SPSCQueue<std::string>> logger_buffer =
                std::make_shared<lockfree::SPSCQueue<std::string>>(20);
        std::shared_ptr<lockfree::SPSCQueue<std::string>> analyzer_buffer =
                std::make_shared<lockfree::SPSCQueue<std::string>>(30);
        std::shared_ptr<lockfree::SPSCQueue<std::string>> printer_buffer =
                std::make_shared<lockfree::SPSCQueue<std::string>>(30);

        /// Creating reader thread
        Reader reader(logger_buffer, analyzer_buffer);

        /// Creating the analyzer thread
        Analyzer analyzer(logger_buffer, printer_buffer,
                          analyzer_buffer);

        /// Creating the printer thread
        Printer printer(logger_buffer, printer_buffer);

        /// Creating the logger thread
        Logger logger(logger_buffer, "/home/draco/logs.txt");

        /// Register signal handler
        std::signal(SIGINT, signal_handler);

        /// Starting the threads
        reader.start();
        analyzer.start();
        printer.start();
        logger.start();

        /// Wait until a termination flag is set
        while (!terminate) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        /// Stopping the threads
        reader.stop();
        analyzer.stop();
        printer.stop();
        logger.stop();
    }
}
BENCHMARK(BM_Main);

BENCHMARK_MAIN();
