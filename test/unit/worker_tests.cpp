#include <catch.hpp>
#include <worker.hpp>
#include <unsafe_fifo_queue.hpp>
#include <functional>
#include <barrier.hpp>
#include <infinite_waiting_strategy.hpp>
#include "spy_thread.h"
#include "test_configuration.h"


SCENARIO("worker can be started and stopped", "[concurrent::worker]") {
    GIVEN("a worker") {
        concurrent::unsafe_fifo_queue<std::function<void(void)>> task_queue;
        std::mutex queue_mutex;
        std::condition_variable queue_empty;
        std::condition_variable queue_not_empty;
        std::condition_variable worker_exited;
        concurrent::semaphore semaphore(0u);

        concurrent::worker<decltype(task_queue), concurrent::infinite_waiting_strategy, concurrent::spy_thread> worker(
                task_queue,
                queue_mutex,
                queue_not_empty,
                queue_empty,
                worker_exited,
                semaphore
        );

        WHEN("nothing else happens") {
            THEN("worker shouldn't be running") {
                REQUIRE_FALSE(worker.running());
            }

            THEN("created thread shouldn't be executing any code") {
                REQUIRE_FALSE(concurrent::spy_thread::alive_threads.front()->joinable());
            }
        }

        WHEN("worker is started") {
            worker.start();

            THEN("worker should be running") {
                REQUIRE(worker.running());
            }

            THEN("created thread should be running") {
                REQUIRE(concurrent::spy_thread::alive_threads.front()->joinable());
            }
        }

        WHEN("worker is started and stopped") {
            worker.start();
            worker.stop();

            THEN("worker shouldn't be running") {
                REQUIRE_FALSE(worker.running());
            }

            // stopped worker requires wakeup to avoid deadlock
            queue_not_empty.notify_one();
        }

        WHEN("worker is moved before start") {
            auto other_worker(std::move(worker));

            THEN("original worker shouldn't be running") {
                REQUIRE_FALSE(worker.running());
            }

            THEN("other worker shouldn't be running") {
                REQUIRE_FALSE(other_worker.running());
            }

            THEN("no thread should be executing any code") {
                for (auto thread: concurrent::spy_thread::alive_threads) {
                    REQUIRE_FALSE(thread->joinable());
                }
            }
        }

        if (worker.running()) {
            worker.stop();
            queue_not_empty.notify_one();
        }
    }
}

SCENARIO("a started worker should execute tasks", "[concurrent::worker]") {
    GIVEN("a worker") {
        concurrent::unsafe_fifo_queue<std::function<void(void)>> task_queue;
        std::mutex queue_mutex;
        std::condition_variable queue_empty;
        std::condition_variable queue_not_empty;
        std::condition_variable worker_exited;
        concurrent::semaphore semaphore(0u);

        concurrent::worker<decltype(task_queue), concurrent::infinite_waiting_strategy, concurrent::spy_thread> worker(
                task_queue,
                queue_mutex,
                queue_not_empty,
                queue_empty,
                worker_exited,
                semaphore
        );

        worker.start();

        WHEN("task is added to queue and worker is notified") {
            std::unique_lock<std::mutex> lock(queue_mutex);
            auto barrier = std::make_shared<concurrent::barrier>(2);

            task_queue.push(
                    [barrier] {
                        barrier->wait();
                    }
            );

            lock.unlock();

            queue_not_empty.notify_one();

            THEN("the task should be finally completed") {
                REQUIRE(barrier->wait_for(config::default_timeout));
            }
        }

        WHEN("many tasks are added to queue and worker is notified") {
            const unsigned tasks_count = 4;
            auto barriers = std::make_shared<std::vector<concurrent::barrier>>();

            for (auto i = 0u; i < tasks_count; ++i) {
                barriers->emplace_back(2);
            }

            std::unique_lock<std::mutex> lock(queue_mutex);

            for (auto i = 0u; i < tasks_count; ++i) {
                task_queue.push(
                        [barriers, i] {
                            barriers->at(i).wait();
                        }
                );
            }

            lock.unlock();

            queue_not_empty.notify_one();

            THEN("all tasks should be completed in order") {
                for (auto i = 0u; i < tasks_count; ++i) {
                    INFO("Barrier number " + std::to_string(i));
                    REQUIRE(barriers->at(i).wait_for(config::default_timeout));
                }
            }
        }

        WHEN("worker is stopped") {
            worker.stop();

            AND_WHEN("task is added to queue and worker is notified") {
                std::unique_lock<std::mutex> lock(queue_mutex);
                auto barrier = std::make_shared<concurrent::barrier>(2);

                task_queue.push(
                        [barrier] {
                            barrier->wait();
                        }
                );

                lock.unlock();

                queue_not_empty.notify_one();

                THEN("the task shouldn't be completed") {
                    REQUIRE_FALSE(barrier->wait_for(std::chrono::milliseconds(100)));
                }
            }
        }

        if (worker.running()) {
            worker.stop();
            queue_not_empty.notify_one();
        }
    }
}