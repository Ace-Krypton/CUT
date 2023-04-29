#ifndef CUT_TBUFFER_HPP
#define CUT_TBUFFER_HPP
#pragma once

#include <mutex>
#include <vector>
#include <memory>
#include <condition_variable>

template<class T>
class TBuffer {
public:
    explicit TBuffer(std::size_t size) : _buffer(size), _count(0), _head(0), _tail(0) { };

    auto push(T value) -> void;
    auto pop() -> std::unique_ptr<T>;
    auto empty() -> bool;

private:
    std::mutex _mutex;
    std::size_t _head;
    std::size_t _tail;
    std::size_t _count;
    std::condition_variable _cond_full;
    std::condition_variable _cond_empty;
    std::vector<std::unique_ptr<T>> _buffer;
};

#endif //CUT_TBUFFER_HPP
