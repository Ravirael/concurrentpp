#include <catch.hpp>
#include <task_queues.hpp>
#include <parallel_for_each.hpp>

SCENARIO("basic parrallel_for_each usage", "[concurrent::parallel_for_each]") {
    GIVEN("a 4-threaded task queue") {
        concurrent::n_threaded_fifo_task_queue task_queue(4);

        GIVEN("vector with 1, 2, 3, 4") {
            std::vector<int> vector{1, 2, 3, 4};

            WHEN("parallel_for_each is called with task constructor") {
                auto task_constructor = [](int &i) {
                    i *= i;
                };
                concurrent::parallel_for_each(task_queue, vector.begin(), vector.end(), task_constructor);

                THEN("tasks are finished") {
                    REQUIRE(vector == std::vector<int>{1, 4, 9, 16});
                }
            }
        }

    }
}
