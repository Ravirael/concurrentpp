#include <iostream>
#include <chrono>
#include <task_queues.hpp>
#include <atomic>
#include <fstream>
#include <zconf.h>
#include "lifetime_logger.h"

using namespace std::chrono_literals;

const auto sleep_time = 100us;

unsigned performUsingDynamicTaskQueue(unsigned count) {
    lifetime_logger logger("Atomic incrementation threaded thread pool: ");
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
    lifetime_logger logger("Atomic incrementation using n-threaded thread pool: ");
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
    lifetime_logger logger("Atomic incrementation using threads: ");
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

void test_atomic_increment() {
    const auto count = 1000u;
    std::cout << performUsingThreads(count)          << std::endl;
    std::cout << performUsingDynamicTaskQueue(count) << std::endl;
    std::cout << performUsingStaticTaskQueue(count)  << std::endl;
}

void write_file() {
    std::ofstream tmp("/tmp/delete_me_" + std::to_string((unsigned)rand()));
    tmp << "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        << "Cras vestibulum, diam placerat porta euismod, mi ipsum lacinia diam, a varius urna ante id ligula."
        << " Maecenas lorem nulla, tincidunt sed cursus vel, sodales ut quam. Aliquam quis convallis mauris. "
        << "In hac habitasse platea dictumst. Integer convallis nibh vel orci mattis, eget interdum ex ultricies. "
        << "Quisque lacinia neque at tellus bibendum, sit amet commodo nisi mollis. "
        << "Aliquam finibus diam quis ipsum tincidunt, et malesuada nibh placerat. Curabitur tincidunt elit eget orci "
        << "convallis, non tristique sem sodales. Phasellus tempor sed neque id condimentum. Aliquam tortor metus, "
        << "tincidunt eget tortor non, porttitor accumsan nisl. Mauris laoreet condimentum ante, a placerat lacus "
        << "mollis laoreet. Nullam tristique ut sapien in egestas. Curabitur dui ipsum, euismod in sem non, "
        << "eleifend accumsan metus.";
}

void perform_io_dynamic_queue(unsigned count) {
    lifetime_logger logger("File writing using dynamic threaded thread pool: ");

    {
        concurrent::dynamic_fifo_task_queue queue(16, 32);

        for (auto i = 0u; i < count; ++i) {
            queue.push(
                    write_file
            );
        }
    }
}

void perform_io_static_task_queue(unsigned count) {
    lifetime_logger logger("File writing using n-threaded thread pool: ");
    {
        concurrent::n_threaded_fifo_task_queue queue(16);

        for (auto i = 0u; i < count; ++i) {
            queue.push(
                    write_file
            );
        }
    }
}

void perform_io_threads(unsigned count) {
    lifetime_logger logger("File writing using threads: ");
    std::vector<std::thread> threads;

    threads.reserve(count);

    for (auto i = 0u; i < count; ++i) {
        threads.emplace_back(
                write_file
        );
    }

    for (auto &thread: threads) {
        thread.join();
    }
}

void test_writing_file() {
    constexpr auto count = 1000u;
    perform_io_threads(count);
    perform_io_dynamic_queue(count);
    perform_io_static_task_queue(count);
}

int main() {
    test_atomic_increment();
    test_writing_file();
}