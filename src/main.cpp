#include <thread>
#include <string>
#include <iostream>

#include "../include/TBuffer.hpp"

auto producer(TBuffer<std::string>& buffer, std::size_t num_items) -> void {
    for (std::size_t i = 0; i < num_items; ++i) {
        std::string item = "Item " + std::to_string(i);
        buffer.push(item);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

auto consumer(TBuffer<std::string>& buffer, std::size_t num_items) -> void {
    for (std::size_t i = 0; i < num_items; ++i) {
        std::unique_ptr<std::string> item = buffer.pop();
        std::cout << "Consumed item: " << *item << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

auto main() -> int {
    TBuffer<std::string> buffer(5);

    std::thread producer_thread(producer, std::ref(buffer), 10);
    std::thread consumer_thread(consumer, std::ref(buffer), 10);

    producer_thread.join();
    consumer_thread.join();

    return 0;
}
