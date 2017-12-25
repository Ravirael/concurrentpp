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
                concurrent::unsafe_priority_queue<std::function<void(void)>, int>,
                concurrent::spy_thread
        > task_queue(4);

        WHEN("task with different priorities are added") {
            concurrent::barrier barrier(5);

            // add 4 tasks to block all workers
            for (auto i = 0u; i < 4u; ++i) {
                task_queue.emplace(
                        0,
                        [&barrier] {
                            barrier.wait();
                        }
                );
            }

            std::mutex mutex;
            std::vector<concurrent::barrier> barriers;
            std::vector<int> finished_tasks_priorities;
            std::condition_variable last_task_finished;

            for (int i = 0; i < 4; ++i) {
                barriers.emplace_back(4);
            }

            // add 16 tasks with 4 priorieties
            for (auto i = 0u; i < 4u; ++i) {
                for (auto priority = 0; priority < 4; ++priority) {
                    task_queue.emplace(
                            priority,
                            [&mutex, priority, &finished_tasks_priorities, &barriers, &last_task_finished] {
                                {
                                    std::lock_guard<std::mutex> lock(mutex);
                                    finished_tasks_priorities.push_back(priority);
                                }
                                barriers[priority].wait();
                                {
                                    std::lock_guard<std::mutex> lock(mutex);
                                    if (finished_tasks_priorities.size() == 16) {
                                        last_task_finished.notify_one();
                                    }
                                }
                            }
                    );
                }
            }

            // release workers
            barrier.wait();

            THEN("priorities of finished task should be descending") {
                std::unique_lock<std::mutex> lock(mutex);
                REQUIRE(
                        last_task_finished.wait_for(
                                lock,
                                config::default_timeout,
                                [&finished_tasks_priorities] { return finished_tasks_priorities.size() == 16;}
                        )
                );
                for (auto i = 1u; i < finished_tasks_priorities.size(); ++i) {
                    REQUIRE(finished_tasks_priorities[i] <= finished_tasks_priorities[i - 1]);
                }
            }

        }

    }
}