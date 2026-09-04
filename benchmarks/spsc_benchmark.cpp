#include "lfqueue.hpp"

#include <benchmark/benchmark.h>

#include <barrier>
#include <cstddef>
#include <cstdint>
#include <thread>

template <std::size_t Capacity, std::size_t OffsetAlignment>
void BM_SPSC_uint64(benchmark::State& state) {
    constexpr std::uint64_t messages_per_iteration = 1U << 20;
    constexpr std::uint64_t expected_sum =
        messages_per_iteration * (messages_per_iteration - 1) / 2;

    std::uint64_t total_full_retries = 0;
    std::uint64_t total_empty_retries = 0;
    std::uint64_t processed_messages = 0;

    for (auto _ : state) {
        state.PauseTiming();

        LFQueue<std::uint64_t, Capacity, OffsetAlignment> queue;
        std::barrier start_gate{3};
        std::uint64_t full_retries = 0;
        std::uint64_t empty_retries = 0;
        std::uint64_t received_sum = 0;

        {
            std::jthread producer([&] {
                start_gate.arrive_and_wait();

                for (std::uint64_t value = 0; value < messages_per_iteration; ++value) {
                    while (!queue.push(value)) {
                        ++full_retries;
                    }
                }
            });

            std::jthread consumer([&] {
                start_gate.arrive_and_wait();

                std::uint64_t local_sum = 0;

                for (std::uint64_t i = 0; i < messages_per_iteration; ++i) {
                    for (;;) {
                        auto result = queue.pop();

                        if (result) {
                            local_sum += *result;
                            break;
                        }

                        ++empty_retries;
                    }
                }

                received_sum = local_sum;
            });

            state.ResumeTiming();
            start_gate.arrive_and_wait();
        } // jthread destructors join both threads while timing is active.

        state.PauseTiming();

        if (received_sum != expected_sum) {
            state.SkipWithError("Queue lost, duplicated, or corrupted a message");
            break;
        }

        total_full_retries += full_retries;
        total_empty_retries += empty_retries;
        processed_messages += messages_per_iteration;
        benchmark::DoNotOptimize(received_sum);
        state.ResumeTiming();
    }

    state.SetItemsProcessed(static_cast<std::int64_t>(processed_messages));
    state.counters["full_retries"] = static_cast<double>(total_full_retries);
    state.counters["empty_retries"] = static_cast<double>(total_empty_retries);
}

template <std::size_t Capacity>
void BM_SPSC_packed_uint64(benchmark::State& state) {
    BM_SPSC_uint64<Capacity, alignof(std::atomic_size_t)>(state);
}

template <std::size_t Capacity>
void BM_SPSC_align64_uint64(benchmark::State& state) {
    BM_SPSC_uint64<Capacity, 64>(state);
}

template <std::size_t Capacity>
void BM_SPSC_hardware_hint_uint64(benchmark::State& state) {
    BM_SPSC_uint64<Capacity, std::hardware_destructive_interference_size>(state);
}

#define REGISTER_SPSC_LAYOUT_BENCHMARKS(Capacity)                                          \
    BENCHMARK_TEMPLATE(BM_SPSC_packed_uint64, Capacity)->UseRealTime();                     \
    BENCHMARK_TEMPLATE(BM_SPSC_align64_uint64, Capacity)->UseRealTime();                    \
    BENCHMARK_TEMPLATE(BM_SPSC_hardware_hint_uint64, Capacity)->UseRealTime()

REGISTER_SPSC_LAYOUT_BENCHMARKS(64);
REGISTER_SPSC_LAYOUT_BENCHMARKS(1024);
REGISTER_SPSC_LAYOUT_BENCHMARKS(65536);

#undef REGISTER_SPSC_LAYOUT_BENCHMARKS
