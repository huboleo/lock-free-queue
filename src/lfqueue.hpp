#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <expected>
#include <string_view>

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

template <typename T, std::size_t N> class LFQueue {
  public:
    [[nodiscard]] std::expected<void, QueueOperationError> push(T&& data);
    [[nodiscard]] std::expected<void, QueueOperationError> push(const T& data);

    [[nodiscard]] std::expected<T, QueueOperationError> pop();

    [[nodiscard]] constexpr std::size_t capacity() const noexcept { return N; }

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] bool full() const noexcept;

  private:
    std::array<T, N> _buffer{};
    std::atomic_size_t _write_offset{0};
    std::atomic_size_t _read_offset{0};
};
