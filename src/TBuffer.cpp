#include "../include/TBuffer.hpp"

/**
 * @brief Inserts an element into the buffer
 * @param value The value to be inserted
 */
template<class T>
auto TBuffer<T>::push(T value) -> void {
    std::unique_lock<std::mutex> lock(_mutex);
    _cond_full.wait(lock, [this]() { return _count != _buffer.size(); });
    _buffer[_tail] = std::make_unique<T>(std::move(value));
    _tail = (_tail + 1) % _buffer.size();
    ++_count;
    lock.unlock();
    _cond_empty.notify_one();
}

/**
 * @brief Removes an item from the buffer.
 * This function removes an item from the buffer. If the buffer is empty, the calling thread
 * will block until an item is available.
 * @return A unique pointer to the item that was removed from the buffer.
 */
template<class T>
auto TBuffer<T>::pop() -> std::unique_ptr<T> {
    std::unique_lock<std::mutex> lock(_mutex);
    _cond_empty.wait(lock, [this]() { return _count != 0; });
    auto item = std::move(_buffer[_head]);
    _head = (_head + 1) % _buffer.size();
    --_count;
    lock.unlock();
    _cond_full.notify_one();
    return item;
}

template class TBuffer<int>;
template class TBuffer<std::string>;
