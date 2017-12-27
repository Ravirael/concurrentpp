#include <catch.hpp>
#include <n_threaded_task_queue.hpp>
#include <mutex>
#include <unsafe_fifo_queue.hpp>
#include <functional>
#include <barrier.hpp>
#include <dynamic_task_queue.hpp>
#include "spy_thread.h"
#include "test_configuration.h"

SCENARIO("creating dynamic queue, adding and executing tasks", "[concurrent::dynamic_task_queue]") {
    GIVEN("a 4-threaded fifo task queue") {
        concurrent::dynamic_task_queue<
                concurrent::unsafe_fifo_queue<std::function<void(void)>>,
                concurrent::spy_thread
        > task_queue(
                4,
                8,
                std::chrono::milliseconds(10)
        );

        WHEN("nothing else happens") {
            THEN("only cleaning thread should be spawned") {
                REQUIRE(concurrent::spy_thread::alive_threads.size() == 1);
            }

            AND_THEN("all spawned threads should be executing code") {
                for (auto thread: concurrent::spy_thread::alive_threads) {
                    REQUIRE(thread->joinable());
                }
            }
        }

        WHEN("a single task is pushed as r-value") {
            auto barrier = std::make_shared<concurrent::barrier>(2);

            task_queue.push(
                    [barrier] {
                        barrier->wait();
                    }
            );

            THEN("the task should be finally completed and one new thread should be spawned") {
                REQUIRE(concurrent::spy_thread::alive_threads.size() == 2);
                REQUIRE(barrier->wait_for(config::default_timeout));
            }
        }

        WHEN("a single task is pushed as l-value") {
            auto barrier = std::make_shared<concurrent::barrier>(2);

            auto task = [barrier]{
                barrier->wait();
            };

            task_queue.push(task);

            THEN("the task should be finally completed and one new thread should be spawned") {
                REQUIRE(concurrent::spy_thread::alive_threads.size() == 2);
                REQUIRE(barrier->wait_for(config::default_timeout));
            }
        }

        WHEN("4 tasks are pushed") {
            auto barrier = std::make_shared<concurrent::barrier>(5);

            for (auto i = 0u; i < 4u; ++i) {
                task_queue.push(
                  [barrier] {
                      barrier->wait();
                  }
                );
            }

            THEN("4 new threads should be spawned and all tasks should be executed concurrently") {
                REQUIRE(concurrent::spy_thread::alive_threads.size() == 5);
                REQUIRE(barrier->wait_for(config::default_timeout));
            }
        }

        WHEN("task adding another task is pushed") {
            auto barrier = std::make_shared<concurrent::barrier>(2);

            task_queue.push(
                    [&task_queue, barrier] {
                        task_queue.push(
                                [barrier] {
                                    barrier->wait();
                                }
                        );
                    }
            );

            THEN("tasks are finally finished") {
                REQUIRE(barrier->wait_for(config::default_timeout));
            }

        }

        WHEN("8 tasks are pushed") {
            auto first_barrier = std::make_shared<concurrent::barrier>(5);
            auto second_barrier = std::make_shared<concurrent::barrier>(5);

            for (auto i = 0u; i < 4u; ++i) {
                task_queue.push(
                        [first_barrier] {
                            first_barrier->wait();
                        }
                );
            }

            for (auto i = 0u; i < 4u; ++i) {
                task_queue.push(
                        [second_barrier] {
                            second_barrier->wait();
                        }
                );
            }

            THEN("8 threads are spawned") {
                REQUIRE(concurrent::spy_thread::alive_threads.size() == 9);

                AND_WHEN("threads are released") {
                    REQUIRE(second_barrier->wait_for(config::default_timeout));
                    REQUIRE(first_barrier->wait_for(config::default_timeout));

                    AND_WHEN("we wait longer than timeout") {
                        std::this_thread::sleep_for(std::chrono::milliseconds(30));
                        THEN("dynamic threads are killed") {
                            std::lock_guard<std::mutex> lock(concurrent::spy_thread::alive_threads_mutex);
                            REQUIRE(concurrent::spy_thread::alive_threads.size() == 5);
                        }
                    }
                }
            }
        }
    }

}