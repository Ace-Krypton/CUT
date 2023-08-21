#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>

#ifdef __has_cpp_attribute
#if __has_cpp_attribute(nodiscard)
#define RIGTORP_NODISCARD [[nodiscard]]
#endif
#endif
#ifndef RIGTORP_NODISCARD
#define RIGTORP_NODISCARD
#endif

namespace lockfree {
template <typename T, typename Allocator = std::allocator<T>>
class SPSCQueue {
#if defined(__cpp_if_constexpr) && defined(__cpp_lib_void_t)
    template <typename Alloc2, typename = void>
    struct has_allocate_at_least : std::false_type {};

    template <typename Alloc2>
    struct has_allocate_at_least<
        Alloc2, std::void_t<typename Alloc2::value_type,
                            decltype(std::declval<Alloc2 &>().allocate_at_least(
                                size_t{}))>> : std::true_type {};
#endif

public:
    explicit SPSCQueue(const size_t capacity,
                       const Allocator &allocator = Allocator())
        : capacity_(capacity), allocator_(allocator) {
        if (capacity_ < 1) {
            capacity_ = 1;
        }
        capacity_++;
        if (capacity_ > SIZE_MAX - 2 * kPadding) {
            capacity_ = SIZE_MAX - 2 * kPadding;
        }

#if defined(__cpp_if_constexpr) && defined(__cpp_lib_void_t)
        if constexpr (has_allocate_at_least<Allocator>::value) {
            auto res = allocator_.allocate_at_least(capacity_ + 2 * kPadding);
            slots_ = res.ptr;
            capacity_ = res.count - 2 * kPadding;
        } else {
            slots_ = std::allocator_traits<Allocator>::allocate(
                allocator_, capacity_ + 2 * kPadding);
        }
#else
        slots_ = std::allocator_traits<Allocator>::allocate(
            allocator_, capacity_ + 2 * kPadding);
#endif

        static_assert(alignof(SPSCQueue<T>) == kCacheLineSize);
        static_assert(sizeof(SPSCQueue<T>) >= 3 * kCacheLineSize);
        assert(reinterpret_cast<char *>(&readIdx_) -
                   reinterpret_cast<char *>(&writeIdx_) >=
               static_cast<std::ptrdiff_t>(kCacheLineSize));
    }

    ~SPSCQueue() {
        while (front()) {
            pop();
        }
        std::allocator_traits<Allocator>::deallocate(allocator_, slots_,
                                                     capacity_ + 2 * kPadding);
    }

    SPSCQueue(const SPSCQueue &) = delete;
    SPSCQueue &operator=(const SPSCQueue &) = delete;

    template <typename... Args>
    void emplace(Args &&...args) noexcept(
        std::is_nothrow_constructible<T, Args &&...>::value) {
        static_assert(std::is_constructible<T, Args &&...>::value,
                      "T must be constructive with Args&&...");
        auto const writeIdx = writeIdx_.load(std::memory_order_relaxed);
        auto nextWriteIdx = writeIdx + 1;
        if (nextWriteIdx == capacity_) {
            nextWriteIdx = 0;
        }
        while (nextWriteIdx == readIdxCache_) {
            readIdxCache_ = readIdx_.load(std::memory_order_acquire);
        }
        new (&slots_[writeIdx + kPadding]) T(std::forward<Args>(args)...);
        writeIdx_.store(nextWriteIdx, std::memory_order_release);
    }

    template <typename... Args>
    RIGTORP_NODISCARD bool try_emplace(Args &&...args) noexcept(
        std::is_nothrow_constructible<T, Args &&...>::value) {
        static_assert(std::is_constructible<T, Args &&...>::value,
                      "T must be constructive with Args&&...");
        auto const writeIdx = writeIdx_.load(std::memory_order_relaxed);
        auto nextWriteIdx = writeIdx + 1;
        if (nextWriteIdx == capacity_) {
            nextWriteIdx = 0;
        }
        if (nextWriteIdx == readIdxCache_) {
            readIdxCache_ = readIdx_.load(std::memory_order_acquire);
            if (nextWriteIdx == readIdxCache_) {
                return false;
            }
        }
        new (&slots_[writeIdx + kPadding]) T(std::forward<Args>(args)...);
        writeIdx_.store(nextWriteIdx, std::memory_order_release);
        return true;
    }

    [[maybe_unused]] void push(const T &v) noexcept(
        std::is_nothrow_copy_constructible<T>::value) {
        static_assert(std::is_copy_constructible<T>::value,
                      "T must be copy constructive");
        emplace(v);
    }

    template <typename P, typename = typename std::enable_if<
                              std::is_constructible<T, P &&>::value>::type>
    void push(P &&v) noexcept(std::is_nothrow_constructible<T, P &&>::value) {
        emplace(std::forward<P>(v));
    }

    RIGTORP_NODISCARD [[maybe_unused]] bool try_push(const T &v) noexcept(
        std::is_nothrow_copy_constructible<T>::value) {
        static_assert(std::is_copy_constructible<T>::value,
                      "T must be copy constructive");
        return try_emplace(v);
    }

    template <typename P, typename = typename std::enable_if<
                              std::is_constructible<T, P &&>::value>::type>
    [[maybe_unused]] RIGTORP_NODISCARD bool try_push(P &&v) noexcept(
        std::is_nothrow_constructible<T, P &&>::value) {
        return try_emplace(std::forward<P>(v));
    }

    RIGTORP_NODISCARD T *front() noexcept {
        auto const readIdx = readIdx_.load(std::memory_order_relaxed);
        if (readIdx == writeIdxCache_) {
            writeIdxCache_ = writeIdx_.load(std::memory_order_acquire);
            if (writeIdxCache_ == readIdx) {
                return nullptr;
            }
        }
        return &slots_[readIdx + kPadding];
    }

    void pop() noexcept {
        static_assert(std::is_nothrow_destructible<T>::value,
                      "T must be nothrow destructible");
        auto const readIdx = readIdx_.load(std::memory_order_relaxed);
        assert(writeIdx_.load(std::memory_order_acquire) != readIdx);
        slots_[readIdx + kPadding].~T();
        auto nextReadIdx = readIdx + 1;
        if (nextReadIdx == capacity_) {
            nextReadIdx = 0;
        }
        readIdx_.store(nextReadIdx, std::memory_order_release);
    }

    RIGTORP_NODISCARD [[maybe_unused]] size_t size() const noexcept {
        std::ptrdiff_t diff = writeIdx_.load(std::memory_order_acquire) -
                              readIdx_.load(std::memory_order_acquire);
        if (diff < 0) {
            diff += capacity_;
        }
        return static_cast<size_t>(diff);
    }

    RIGTORP_NODISCARD [[maybe_unused]] bool empty() const noexcept {
        return writeIdx_.load(std::memory_order_acquire) ==
               readIdx_.load(std::memory_order_acquire);
    }

    RIGTORP_NODISCARD [[maybe_unused]] size_t capacity() const noexcept {
        return capacity_ - 1;
    }

private:
#ifdef __cpp_lib_hardware_interference_size
    static constexpr size_t kCacheLineSize =
        std::hardware_destructive_interference_size;
#else
    static constexpr size_t kCacheLineSize = 64;
#endif

    static constexpr size_t kPadding = (kCacheLineSize - 1) / sizeof(T) + 1;

private:
    size_t capacity_;
    T *slots_;
#if defined(__has_cpp_attribute) && __has_cpp_attribute(no_unique_address)
    Allocator allocator_ [[no_unique_address]];
#else
    Allocator allocator_;
#endif

    alignas(kCacheLineSize) std::atomic<size_t> writeIdx_ = {0};
    alignas(kCacheLineSize) size_t readIdxCache_ = 0;
    alignas(kCacheLineSize) std::atomic<size_t> readIdx_ = {0};
    alignas(kCacheLineSize) size_t writeIdxCache_ = 0;

    [[maybe_unused]] char padding_[kCacheLineSize - sizeof(writeIdxCache_)]{};
};
}  // namespace lockfree
