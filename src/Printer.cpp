#include "../include/Printer.hpp"

/**
 * @brief Prints the data from the printer buffer.
 */
auto Printer::print_data() -> void {
    /// Continuously analyze data until an exit flag is set
    while(!_exit_flag) {
        /// Push a log message to the logger buffer indicating that data is being printed.
        _logger_buffer->push("Data is being printed");

        /// Wait for data if data stream is empty
        if (_printer_buffer->empty()) {
            std::unique_lock<std::mutex> lock(_mutex);
            _cond_var.wait_for(lock, std::chrono::milliseconds(100));
            continue;
        }

        /// Retrieve the next data string from the data stream
        std::string *cpu_percentage = _printer_buffer->front();

        /// If data string is not empty, then process the data
        if (!cpu_percentage->empty()) {
            /// Print the info to the console by de-referencing it
            std::cout << *cpu_percentage << std::endl;
            /// Update the buffer
            _printer_buffer->pop();
        }
    }
}

/**
 * @brief Prints the "|" according to percentage argument.
 * @param percent Percentage value came from Printer thread.
 * @return final std::string as "|".
 */
[[maybe_unused]] auto Printer::progress_bar(const std::string &percent) -> std::string {
    std::string result = "CPU [";
    int _size = 50;

    int boundaries = static_cast<int>(
            (std::stof(percent) / 100) * static_cast<float>(_size));

    for (std::size_t i = 0; i < _size; ++i){
        if (i <= boundaries) result += "|";
        else result += " ";
    }

    result += percent.substr(0, 4) + "%]";
    return result;
}

/**
 * @brief Endless while loop for updating value and printing it to the screen.
 * Exits when user presses "CTRL+C" or whatever the binding is.
 */
[[maybe_unused]] auto Printer::draw() -> void {
    start_color();
    int xMax;
    [[maybe_unused]] int yMax;
    getmaxyx(stdscr, yMax, xMax);
    WINDOW * sys_win = newwin(17, xMax - 1, 0, 0);
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);

    while(!_exit_flag) {
        //print_data(sys_win);
        wrefresh(sys_win);
        refresh();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

/**
 * @brief Starts the Printer thread.
 *
 * This method creates a new thread and starts the `print_data` method.
 */
auto Printer::start() -> void {
    _thread = std::thread(&Printer::print_data, this);
}

/**
 * @brief Stops the Printer thread.
 *
 * This method sets the `_exit_flag` variable to `true`, notifies the condition variable
 * to wake up the thread, and waits for the thread to join before returning.
 */
auto Printer::stop() -> void {
    _exit_flag.store(true);
    std::unique_lock<std::mutex> lock(_mutex);
    _cond_var.notify_one();
    if (_thread.joinable()) {
        _thread.join();
    }
}
