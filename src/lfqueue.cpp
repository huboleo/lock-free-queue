#include "lfqueue.hpp"
#include <atomic>
#include <cstddef>
#include <expected>
#include <utility>

template <typename T, std::size_t N>
std::expected<void, QueueOperationError> LFQueue<T, N>::push(const T& data) {
    const auto write = _write_offset.load(std::memory_order_relaxed);
    const auto read = _read_offset.load(std::memory_order_acquire);

    if (write - read == N) {
        return std::unexpected(QueueOperationError::QUEUE_FULL);
    }

    // bitmask here works as modulo
    _buffer[write & (N - 1)] = data;
    _write_offset.store(write + 1, std::memory_order_release);
    return {};
}

template <typename T, std::size_t N>
std::expected<void, QueueOperationError> LFQueue<T, N>::push(T&& data) {
    const auto write = _write_offset.load(std::memory_order_relaxed);
    const auto read = _read_offset.load(std::memory_order_acquire);

    if (write - read == N) {
        return std::unexpected(QueueOperationError::QUEUE_FULL);
    }

    // bitmask here works as modulo
    _buffer[write & (N - 1)] = std::move(data);
    _write_offset.store(write + 1, std::memory_order_release);
    return {};
}

template <typename T, std::size_t N> std::expected<T, QueueOperationError> LFQueue<T, N>::pop() {
    const auto read = _read_offset.load(std::memory_order_relaxed);
    const auto write = _write_offset.load(std::memory_order_acquire);

    if (read == write) {
        return std::unexpected(QueueOperationError::QUEUE_EMPTY);
    }

    T item = std::move(_buffer[read & (N - 1)]);

    _read_offset.store(read + 1, std::memory_order_release);

    return item;
}

template <typename T, std::size_t N> bool LFQueue<T, N>::empty() const noexcept {
    const auto read = _read_offset.load(std::memory_order_relaxed);
    const auto write = _write_offset.load(std::memory_order_acquire);

    return read == write;
}

template <typename T, std::size_t N> bool LFQueue<T, N>::full() const noexcept {
    const auto write = _write_offset.load(std::memory_order_relaxed);
    const auto read = _read_offset.load(std::memory_order_acquire);

    return write - read == N;
}
