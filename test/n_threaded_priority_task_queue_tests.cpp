#include <catch.hpp>
#include <n_threaded_task_queue.hpp>
#include <mutex>
#include <functional>
#include <unsafe_priority_queue.hpp>
#include <barrier.hpp>
#include "spy_thread.h"
#include "test_configuration.h"

SCENARIO("creating queue, adding and executing tasks", "[concurrent::n_threaded_priority_task_queue]") {
    GIVEN("a 4-threaded priority task queue") {
        concurrent::n_threaded_task_queue<
                std::mutex,
                concurrent::unsafe_priority_queue<std::function<void(void)>, int>,
                concurrent::spy_thread
        > task_queue(4);

        WHEN("a single task is pushed as r-value") {
            std::timed_mutex mutex;
            std::unique_lock<std::timed_mutex> lock(mutex);

            task_queue.push(
                    std::make_pair(
                            0,
                            [&mutex, &lock]{
                                lock.unlock();
                            }
                    )
            );

            THEN("the task should be finally completed") {
                std::unique_lock<std::timed_mutex> second_lock(mutex, std::defer_lock);
                REQUIRE(second_lock.try_lock_for(config::default_timeout));
            }

            THEN("no new thread should be spawned") {
                REQUIRE(concurrent::spy_thread::alive_threads.size() == 4);
            }

            // task can outlive lock so we need to ensure that it was finished after each THEN section
            std::unique_lock<std::timed_mutex> second_lock(mutex, std::defer_lock);
            REQUIRE(second_lock.try_lock_for(config::default_timeout));
        }

        WHEN("a single task is pushed as l-value") {
            std::timed_mutex mutex;
            std::unique_lock<std::timed_mutex> lock(mutex);
            auto task = [&mutex, &lock]{
                lock.unlock();
            };
            auto pair = std::make_pair(0, task);

            task_queue.push(pair);

            THEN("the task should be finally completed") {
                std::unique_lock<std::timed_mutex> second_lock(mutex, std::defer_lock);
                REQUIRE(second_lock.try_lock_for(config::default_timeout));
            }

            THEN("no new thread should be spawned") {
                REQUIRE(concurrent::spy_thread::alive_threads.size() == 4);
            }

            // task can outlive lock so we need to ensure that it was finished after each THEN section
            std::unique_lock<std::timed_mutex> second_lock(mutex, std::defer_lock);
            REQUIRE(second_lock.try_lock_for(config::default_timeout));
        }

    }
}