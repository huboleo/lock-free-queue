# SPSC Ring-Buffer Queue

A small C++23 experiment implementing a bounded single-producer,
single-consumer (SPSC) queue on top of a ring buffer.

The producer publishes completed writes with a release-store. The consumer
acquires that write index before reading the element, then release-stores its
read index so the producer can reuse the slot. The queue is non-blocking:
`push` reports `QUEUE_FULL` and `pop` reports `QUEUE_EMPTY` instead of waiting.

## Usage

```cpp
#include "lfqueue.hpp"

LFQueue<int, 1024> queue;

if (!queue.push(42)) {
    // The queue is full.
}

if (auto value = queue.pop()) {
    // *value == 42
}
```

`N` is a compile-time capacity and must be a non-zero power of two. The
default index layout separates producer and consumer state with 64-byte
alignment to reduce false sharing.

This implementation is SPSC only: call `push` from one producer thread and
`pop` from one consumer thread. It is not an MPMC queue.

## Build

Build the normal executable without downloading benchmark dependencies:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLFQUEUE_BUILD_BENCHMARKS=OFF
cmake --build build --parallel
```

## Benchmarks

Enable benchmarks to download and build Google Benchmark:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLFQUEUE_BUILD_BENCHMARKS=ON
cmake --build build --target lfqueue_benchmark --parallel
./build/lfqueue_benchmark --benchmark_min_time=2s --benchmark_repetitions=10
```

The benchmark compares packed indexes, 64-byte-separated indexes, and the
standard-library hardware-interference hint at capacities 64, 1,024, and
65,536. Results are hardware-dependent; use the measured result rather than
assuming a particular alignment is fastest.
