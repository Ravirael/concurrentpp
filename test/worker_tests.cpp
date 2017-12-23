#include <catch.hpp>
#include <worker.hpp>
#include <unsafe_fifo_queue.hpp>
#include <functional>


SCENARIO("worker can be started and stopped", "[concurrent::worker]") {
    GIVEN("a worker") {
        concurrent::unsafe_fifo_queue<std::function<void(void)>> task_queue;
        std::mutex queue_mutex;
        std::condition_variable queue_empty;
        std::condition_variable queue_not_empty;

        concurrent::worker<decltype(task_queue), decltype(queue_mutex)> worker(
                task_queue,
                queue_mutex,
                queue_not_empty,
                queue_empty
        );

        WHEN("nothing else happens") {
            THEN("worker shouldn't be running") {
                REQUIRE_FALSE(worker.running());
            }
        }

        WHEN("worker is started") {
            worker.start();

            THEN("worker should be running") {
                REQUIRE(worker.running());
            }
        }

        WHEN("worker is started and stopped") {
            worker.start();
            worker.stop();

            THEN("worker shouldn't be running") {
                REQUIRE_FALSE(worker.running());

                // stopped worker requires explicit wakeup to avoid deadlock
                queue_not_empty.notify_one();
            }
        }

        WHEN("worker is moved before start") {
            auto other_worker(std::move(worker));

            THEN("neither of workers should be running") {
                REQUIRE_FALSE(worker.running());
                REQUIRE_FALSE(other_worker.running());
            }
        }

        WHEN("worker is moved after start") {
            worker.start();
            auto other_worker(std::move(worker));

            THEN("original worker shouldn't be running, the other should be running") {
                REQUIRE_FALSE(worker.running());
                REQUIRE(other_worker.running());
            }
        }
    }
}

SCENARIO("a started worker should execute tasks", "[concurrent::worker]") {
    GIVEN("a worker") {
        concurrent::unsafe_fifo_queue<std::function<void(void)>> task_queue;
        std::mutex queue_mutex;
        std::condition_variable queue_empty;
        std::condition_variable queue_not_empty;

        concurrent::worker<decltype(task_queue), decltype(queue_mutex)> worker(
                task_queue,
                queue_mutex,
                queue_not_empty,
                queue_empty
        );

        worker.start();

        WHEN("task is added to queue and worker is notified") {
            std::timed_mutex mutex;
            std::unique_lock<std::timed_mutex> lock(mutex);

            task_queue.push(
                    [&lock] {
                        lock.unlock();
                    }
            );

            queue_not_empty.notify_one();

            THEN("the task should be finally completed") {
                std::unique_lock<std::timed_mutex> second_lock(mutex, std::defer_lock);
                REQUIRE(second_lock.try_lock_for(std::chrono::milliseconds(1000)));
            }
        }

        WHEN("many tasks are added to queue and worker is notified") {
            const unsigned tasks_count = 4;
            std::atomic_uint counter{0};

            for (auto i = 0u; i < 4; ++i) {
                task_queue.push(
                        [&counter] {
                            ++counter;
                        }
                );
            }

            queue_not_empty.notify_one();

            THEN("all tasks should be finally completed") {
                while (counter != tasks_count) {
                    //TODO: some fallback mechanism?
                }
            }
        }

        WHEN("worker is stopped") {
            worker.stop();

            AND_WHEN("task is added to queue and worker is notified") {
                std::timed_mutex mutex;
                std::unique_lock<std::timed_mutex> lock(mutex);

                task_queue.push(
                        [&lock] {
                            lock.unlock();
                        }
                );

                queue_not_empty.notify_one();

                THEN("the task shouldn't be completed") {
                    std::unique_lock<std::timed_mutex> second_lock(mutex, std::defer_lock);
                    REQUIRE_FALSE(second_lock.try_lock_for(std::chrono::milliseconds(100)));
                }
            }
        }
    }
}