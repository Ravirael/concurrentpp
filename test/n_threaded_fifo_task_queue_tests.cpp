#include <catch.hpp>
#include <n_threaded_task_queue.hpp>
#include <mutex>
#include <unsafe_fifo_queue.hpp>
#include <functional>
#include "spy_thread.h"

SCENARIO("adding and executing tasks", "[concurrent::n_threaded_task_queue]") {
    GIVEN("a 4-threaded fifo task queue") {
        concurrent::n_threaded_task_queue<
                std::mutex,
                concurrent::unsafe_fifo_queue<std::function<void(void)>>,
                concurrent::spy_thread
        > task_queue(4);

        WHEN("nothing else happens") {
            THEN("4 threads should be spawned") {
                REQUIRE(concurrent::spy_thread::alive_threads.size() == 4);
            }

            THEN("all spawned threads should be executing code") {
                for (auto thread: concurrent::spy_thread::alive_threads) {
                    REQUIRE(thread->joinable());
                }
            }
        }

        WHEN("a single task is pushed as r-value") {
            std::timed_mutex mutex;
            std::unique_lock<std::timed_mutex> lock(mutex);

            task_queue.push(
                    [&mutex, &lock]{
                        lock.unlock();
                    }
            );

            THEN("the task should be finally completed") {
                std::unique_lock<std::timed_mutex> second_lock(mutex, std::defer_lock);
                REQUIRE(second_lock.try_lock_for(std::chrono::milliseconds(1000)));
            }
        }

        WHEN("a single task is pushed as l-value") {
            std::timed_mutex mutex;
            std::unique_lock<std::timed_mutex> lock(mutex);
            auto task = [&mutex, &lock]{
                lock.unlock();
            };

            task_queue.push(task);

            THEN("the task should be finally completed") {
                std::unique_lock<std::timed_mutex> second_lock(mutex, std::defer_lock);
                REQUIRE(second_lock.try_lock_for(std::chrono::milliseconds(1000)));
            }
        }
    }

}