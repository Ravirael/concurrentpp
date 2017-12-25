#include <catch.hpp>
#include <n_threaded_task_queue.hpp>
#include <mutex>
#include <unsafe_fifo_queue.hpp>
#include <functional>
#include <barrier.hpp>
#include "spy_thread.h"
#include "test_configuration.h"

SCENARIO("creating priority queue, adding and executing tasks", "[concurrent::n_threaded_task_queue]") {
    GIVEN("a 4-threaded fifo task queue") {
        concurrent::n_threaded_task_queue<
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
            concurrent::barrier barrier(2);

            task_queue.push(
                    [&barrier] {
                        barrier.wait();
                    }
            );

            THEN("the task should be finally completed") {
                REQUIRE(barrier.wait_for(config::default_timeout));
            }

            THEN("no new thread should be spawned") {
                REQUIRE(concurrent::spy_thread::alive_threads.size() == 4);
            }

            // task can outlive barrier so we need to ensure that it was finished after each THEN section
            REQUIRE(barrier.wait_for(config::default_timeout));
        }

        WHEN("a single task is pushed as l-value") {
            concurrent::barrier barrier(2);

            auto task = [&barrier]{
                barrier.wait();
            };

            task_queue.push(task);

            THEN("the task should be finally completed") {
                REQUIRE(barrier.wait_for(config::default_timeout));
            }

            THEN("no new thread should be spawned") {
                REQUIRE(concurrent::spy_thread::alive_threads.size() == 4);
            }

            // task can outlive barrier so we need to ensure that it was finished after each THEN section
            REQUIRE(barrier.wait_for(config::default_timeout));
        }

        WHEN("4 tasks are pushed") {
            concurrent::barrier barrier(5);

            for (auto i = 0u; i < 4u; ++i) {
                task_queue.push(
                  [&barrier] {
                      barrier.wait();
                  }
                );
            }

            THEN("all should be executed concurrently") {
                REQUIRE(barrier.wait_for(config::default_timeout));
            }

            // task can outlive barrier so we need to ensure that it was finished after each THEN section
            REQUIRE(barrier.wait_for(config::default_timeout));
        }

        WHEN("8 long running tasks are pushed") {
            for (auto i = 0u; i < 8u; ++i) {
                task_queue.push(
                        [] {
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
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