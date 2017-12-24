#include <catch.hpp>
#include <n_threaded_task_queue.hpp>
#include <mutex>
#include <unsafe_fifo_queue.hpp>
#include <functional>
#include "spy_thread.h"

SCENARIO("creating queue, adding and executing tasks", "[concurrent::n_threaded_task_queue]") {
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

            THEN("no new thread should be spawned") {
                REQUIRE(concurrent::spy_thread::alive_threads.size() == 4);
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

            THEN("no new thread should be spawned") {
                REQUIRE(concurrent::spy_thread::alive_threads.size() == 4);
            }
        }

        WHEN("4 tasks are pushed") {
            std::atomic_uint counter{4u};
            std::timed_mutex mutex;
            std::unique_lock<std::timed_mutex> lock(mutex);

            for (auto i = 0u; i < 4u; ++i) {
                task_queue.push(
                  [&counter, &lock] {
                      --counter;
                      while (counter > 0) {
                          // do nothing
                          // dumb, active synchronization barrier
                      }

                      // to indicate end of waiting
                      if (lock.owns_lock()) {
                          lock.unlock();
                      }
                  }
                );
            }

            THEN("all should be executed concurrently") {
                std::unique_lock<std::timed_mutex> second_lock(mutex, std::defer_lock);
                REQUIRE(second_lock.try_lock_for(std::chrono::milliseconds(1000)));
            }
        }

        WHEN("8 long running tasks are pushed") {
            std::atomic_uint counter{8};
            std::timed_mutex mutex;
            std::unique_lock<std::timed_mutex> lock(mutex);

            for (auto i = 0u; i < 8u; ++i) {
                task_queue.push(
                        [&counter, &lock] {
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                            counter--;

                            if (counter == 0 && lock.owns_lock()) {
                                lock.unlock();
                            }
                        }
                );
            }

            AND_WHEN("wait_until_is_empty is called") {
                task_queue.wait_until_is_empty();

                THEN("the queue is empty") {
                    REQUIRE(task_queue.empty());
                }

                THEN("queue's size is 0") {
                    REQUIRE(task_queue.size() == 0);
                }

                THEN("all tasks are finally completed") {
                    std::unique_lock<std::timed_mutex> second_lock(mutex, std::defer_lock);
                    REQUIRE(second_lock.try_lock_for(std::chrono::milliseconds(1000)));
                }
            }

            AND_WHEN("clear is called") {
                task_queue.clear();

                THEN("the queue is empty") {
                    REQUIRE(task_queue.empty());
                }

                THEN("queue's size is 0") {
                    REQUIRE(task_queue.size() == 0);
                }
            }
        }

    }


}