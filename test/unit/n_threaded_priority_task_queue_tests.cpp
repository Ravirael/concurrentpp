#include <catch.hpp>
#include <n_threaded_task_queue.hpp>
#include <mutex>
#include <functional>
#include <unsafe_priority_queue.hpp>
#include <barrier.hpp>
#include <priority_task_queue_extension.hpp>
#include "spy_thread.h"
#include "test_configuration.h"

SCENARIO("creating priority task queue, adding and executing tasks", "[concurrent::n_threaded_priority_task_queue]") {
    GIVEN("a 4-threaded priority task queue") {
        concurrent::priority_task_queue_extension<
            concurrent::n_threaded_task_queue<
                    concurrent::unsafe_priority_queue<int, std::function<void(void)>>,
                    concurrent::spy_thread
            >
        > task_queue(4);

        WHEN("task with different priorities are added") {
            auto barrier = std::make_unique<concurrent::barrier>(5);

            // add 4 tasks to block all workers
            for (auto i = 0u; i < 4u; ++i) {
                task_queue.emplace(
                        10,
                        [&barrier] {
                            barrier->wait();
                        }
                );
            }

            auto mutex = std::make_shared<std::mutex>();
            auto barriers = std::make_shared<std::vector<concurrent::barrier>>();
            auto finished_tasks_priorities = std::make_shared<std::vector<int>>();
            auto last_task_finished = std::make_shared<std::condition_variable>();

            for (int i = 0; i < 4; ++i) {
                barriers->emplace_back(4);
            }

            // add 16 tasks with 4 priorieties
            for (auto i = 0u; i < 4u; ++i) {
                for (auto priority = 0; priority < 4; ++priority) {
                    task_queue.emplace(
                            priority,
                            [mutex, priority, finished_tasks_priorities, barriers, last_task_finished] {
                                {
                                    std::lock_guard<std::mutex> lock(*mutex);
                                    finished_tasks_priorities->push_back(priority);
                                }
                                barriers->at(priority).wait();
                                bool notify;
                                {
                                    std::lock_guard<std::mutex> lock(*mutex);
                                    notify = (finished_tasks_priorities->size() == 16);
                                }
                                if (notify) {
                                    last_task_finished->notify_one();
                                }
                            }
                    );
                }
            }

            // release workers
            REQUIRE(barrier->wait_for(config::default_timeout));

            THEN("priorities of finished task should be descending") {
                std::unique_lock<std::mutex> lock(*mutex);
                REQUIRE(
                        last_task_finished->wait_for(
                                lock,
                                config::default_timeout,
                                [finished_tasks_priorities] { return finished_tasks_priorities->size() == 16;}
                        )
                );
                for (auto i = 1u; i < finished_tasks_priorities->size(); ++i) {
                    REQUIRE(finished_tasks_priorities->at(i) <= finished_tasks_priorities->at(i - 1));
                }
            }

        }

        WHEN("task with result is pushed") {
            auto result = task_queue.push_with_result(std::make_pair(0, []{return 1;}));

            THEN("the task is finally finished") {
                REQUIRE(result.get() == 1);
            }
        }

        WHEN("task with result is emplaced") {
            auto result = task_queue.emplace_with_result(0, []{return 1;});

            THEN("the task is finally finished") {
                REQUIRE(result.get() == 1);
            }
        }

    }
}