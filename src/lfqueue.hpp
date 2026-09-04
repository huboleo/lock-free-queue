#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <expected>
#include <new>
#include <string_view>
#include <utility>

enum class QueueOperationError {
    QUEUE_FULL,
    QUEUE_EMPTY,
};

[[nodiscard]] constexpr std::string_view
queue_operation_error_to_string(QueueOperationError error) noexcept {
    switch (error) {
    case QueueOperationError::QUEUE_FULL:
        return "Queue is full";
    case QueueOperationError::QUEUE_EMPTY:
        return "Queue is empty";
    }

    return "Unknown queue error";
}

template <typename T, std::size_t N, std::size_t OffsetAlignment = 64> class LFQueue {
    static_assert(N > 0, "Buffer size must be greater than 0");
    static_assert((N & (N - 1)) == 0, "Buffer size must be a power of two");
    static_assert(OffsetAlignment >= alignof(std::atomic_size_t),
                  "Offset alignment must satisfy atomic_size_t alignment");

  public:
    [[nodiscard]] std::expected<void, QueueOperationError> push(T&& data);
    [[nodiscard]] std::expected<void, QueueOperationError> push(const T& data);

    [[nodiscard]] std::expected<T, QueueOperationError> pop();

    [[nodiscard]] constexpr std::size_t capacity() const noexcept { return N; }

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] bool full() const noexcept;

  private:
    std::array<T, N> _buffer{};
    struct alignas(OffsetAlignment) ProducerState {
        std::atomic_size_t write_offset{0};
    };

    struct alignas(OffsetAlignment) ConsumerState {
        std::atomic_size_t read_offset{0};
    };

    ProducerState producer_;
    ConsumerState consumer_;
};

template <typename T, std::size_t N, std::size_t OffsetAlignment>
std::expected<void, QueueOperationError> LFQueue<T, N, OffsetAlignment>::push(const T& data) {
    const auto write = producer_.write_offset.load(std::memory_order_relaxed);
    const auto read = consumer_.read_offset.load(std::memory_order_acquire);

    if (write - read == N) {
        return std::unexpected(QueueOperationError::QUEUE_FULL);
    }

    // bitmask here works as modulo
    _buffer[write & (N - 1)] = data;
    producer_.write_offset.store(write + 1, std::memory_order_release);
    return {};
}

template <typename T, std::size_t N, std::size_t OffsetAlignment>
std::expected<void, QueueOperationError> LFQueue<T, N, OffsetAlignment>::push(T&& data) {
    const auto write = producer_.write_offset.load(std::memory_order_relaxed);
    const auto read = consumer_.read_offset.load(std::memory_order_acquire);

    if (write - read == N) {
        return std::unexpected(QueueOperationError::QUEUE_FULL);
    }

    // bitmask here works as modulo
    _buffer[write & (N - 1)] = std::move(data);
    producer_.write_offset.store(write + 1, std::memory_order_release);
    return {};
}

template <typename T, std::size_t N, std::size_t OffsetAlignment>
std::expected<T, QueueOperationError> LFQueue<T, N, OffsetAlignment>::pop() {
    const auto read = consumer_.read_offset.load(std::memory_order_relaxed);
    const auto write = producer_.write_offset.load(std::memory_order_acquire);

    if (read == write) {
        return std::unexpected(QueueOperationError::QUEUE_EMPTY);
    }

    T item = std::move(_buffer[read & (N - 1)]);

    consumer_.read_offset.store(read + 1, std::memory_order_release);

    return item;
}

template <typename T, std::size_t N, std::size_t OffsetAlignment>
bool LFQueue<T, N, OffsetAlignment>::empty() const noexcept {
    const auto read = consumer_.read_offset.load(std::memory_order_relaxed);
    const auto write = producer_.write_offset.load(std::memory_order_acquire);

    return read == write;
}

template <typename T, std::size_t N, std::size_t OffsetAlignment>
bool LFQueue<T, N, OffsetAlignment>::full() const noexcept {
    const auto write = producer_.write_offset.load(std::memory_order_relaxed);
    const auto read = consumer_.read_offset.load(std::memory_order_acquire);

    return write - read == N;
}
