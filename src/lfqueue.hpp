#include <array>
#include <cstddef>

template <std::size_t BufferSize> class LFQueue {
  public:
    LFQueue() = default;

  private:
    std::array<std::byte, BufferSize> _buffer{};
};
