#include <iostream>
#include <chrono>
#include <task_queues.hpp>
#include <atomic>
#include "lifetime_logger.h"

using namespace std::chrono_literals;

const auto sleep_time = 1000us;

unsigned performUsingDynamicTaskQueue(unsigned count) {
    lifetime_logger logger("Using dynamic threaded thread pool: ");
    std::atomic_uint atomic{0};

    {
        concurrent::dynamic_fifo_task_queue queue(16, 32);

        for (auto i = 0u; i < count; ++i) {
            queue.push(
                    [&atomic] {
                        std::this_thread::sleep_for(sleep_time);
                        ++atomic;
                    }
            );
        }
    }

    return atomic.load();
}

unsigned performUsingStaticTaskQueue(unsigned count) {
    lifetime_logger logger("Using n-threaded thread pool: ");
    std::atomic_uint atomic{0};

    {
        concurrent::n_threaded_fifo_task_queue queue(16);

        for (auto i = 0u; i < count; ++i) {
            queue.push(
                    [&atomic] {
                        std::this_thread::sleep_for(sleep_time);
                        ++atomic;
                    }
            );
        }
    }

    return atomic.load();
}

unsigned performUsingThreads(unsigned count) {
    lifetime_logger logger("Using threads: ");
    std::atomic_uint atomic{0};
    std::vector<std::thread> threads;

    threads.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        threads.emplace_back(
                [&atomic] {
                    std::this_thread::sleep_for(sleep_time);
                    ++atomic;
                }
        );
    }

    for (auto &thread: threads) {
        thread.join();
    }

    return atomic.load();
}

int main() {
    const auto count = 1000u;
    std::cout << performUsingThreads(count)          << std::endl;
    std::cout << performUsingDynamicTaskQueue(count) << std::endl;
    std::cout << performUsingStaticTaskQueue(count)  << std::endl;
}