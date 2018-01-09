#include <catch.hpp>
#include <n_threaded_task_queue.hpp>
#include <mutex>
#include <unsafe_fifo_queue.hpp>
#include <functional>
#include <barrier.hpp>
#include <task_queue_extension.hpp>
#include "spy_thread.h"
#include "test_configuration.h"

SCENARIO("creating task queue, adding and executing tasks", "[concurrent::n_threaded_task_queue]") {
    GIVEN("a 4-threaded fifo task queue") {
        concurrent::task_queue_extension<
                concurrent::n_threaded_task_queue<
                    concurrent::unsafe_fifo_queue<
                            std::function<void(void)>
                    >,
                    concurrent::spy_thread
                >
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
            auto barrier = std::make_shared<concurrent::barrier>(2);

            task_queue.push(
                    [barrier] {
                        barrier->wait();
                    }
            );

            THEN("the task should be finally completed and no new thread should be spawned") {
                REQUIRE(barrier->wait_for(config::default_timeout));
                REQUIRE(concurrent::spy_thread::alive_threads.size() == 4);
            }
        }

        WHEN("a single task is pushed as l-value") {
            auto barrier = std::make_shared<concurrent::barrier>(2);

            auto task = [barrier]{
                barrier->wait();
            };

            task_queue.push(task);

            THEN("the task should be finally completed and no new thread should be spawned") {
                REQUIRE(concurrent::spy_thread::alive_threads.size() == 4);
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

            THEN("all should be executed concurrently") {
                REQUIRE(barrier->wait_for(config::default_timeout));
            }
        }

        WHEN("task with result is pushed") {
            std::function<int()> foo = []{return 4;};
            auto result = task_queue.push_with_result(foo);

            THEN("task should finally be executed") {
                REQUIRE(result.get() == 4);
            }
        }

        WHEN("8 long running tasks are pushed") {
            for (auto i = 0u; i < 8u; ++i) {
                task_queue.push(
                        [] {
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        }
                );
            }

            THEN("number of threads should not increase") {
                REQUIRE(concurrent::spy_thread::alive_threads.size() == 4);
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

        WHEN("16 tasks are added") {
            auto counter = std::make_shared<std::atomic_uint>(0);
            for (int i = 0; i < 16; ++i) {
                task_queue.push(
                        [counter] {
                            std::this_thread::sleep_for(1ms);
                            (*counter)++;
                        }
                );
            }

            AND_WHEN("`wait_for_finishing_tasks` is called") {
                task_queue.wait_for_finishing_tasks();

                THEN("all task are finished") {
                    REQUIRE(*counter == 16);
                }
            }
        }
    }

    GIVEN("a 4-threaded fifo task queue filled with tasks") {
        auto barrier = std::make_shared<concurrent::barrier>(5);
        concurrent::n_threaded_task_queue<
                concurrent::unsafe_fifo_queue<std::function<void(void)>>,
                concurrent::spy_thread
        > task_queue(
                4,
                concurrent::unsafe_fifo_queue<std::function<void(void)>>({
                        [barrier]{barrier->wait();},
                        [barrier]{barrier->wait();},
                        [barrier]{barrier->wait();},
                        [barrier]{barrier->wait();}
                })
        );

        WHEN("nothing else happens") {
            THEN("all task should finish") {
                REQUIRE(barrier->wait_for(config::default_timeout));
            }
        }
    }

}