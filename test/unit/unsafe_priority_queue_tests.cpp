#include <catch.hpp>
#include <unsafe_priority_queue.hpp>
#include <memory>

namespace {
    enum class Priority {
        low, normal, high
    };
}

TEST_CASE("different ways of pushing values priority queue of simple types", "[concurrent::unsafe_priority_queue]") {
    concurrent::unsafe_priority_queue<Priority, int> queue;

    SECTION("pushing l-value should work") {
        const auto pair = std::make_pair(Priority::normal, 1);

        queue.push(pair);

        REQUIRE(queue.pop() == 1);
    }

    SECTION("emplacing should work") {
        queue.emplace(Priority::normal, 1);

        REQUIRE(queue.pop() == 1);
    }
}

TEST_CASE("different ways of pushing values to priority queue of only move-constructable types", "[concurrent::unsafe_priority_queue]") {
    concurrent::unsafe_priority_queue<std::unique_ptr<Priority>, std::unique_ptr<int>> queue;

    SECTION("emplacing should work") {
        queue.emplace(
                std::make_unique<Priority>(Priority::normal),
                std::make_unique<int>(1)
        );

        REQUIRE(*queue.pop() == 1);
    }
}

SCENARIO("basic unsafe priority queue operations", "[concurrent::unsafe_priority_queue") {
    GIVEN("unsafe priority queue with ints as priorities and as values") {
        concurrent::unsafe_priority_queue<Priority, int> queue;

        WHEN("one value is pushed") {
            queue.emplace(Priority::normal, 1);

            THEN("that value is popped") {
                REQUIRE(queue.pop() == 1);
            }
        }

        WHEN("three values with different priorities are added") {
            queue.emplace(Priority::normal, 1);
            queue.emplace(Priority::low, 0);
            queue.emplace(Priority::high, 2);

            THEN("values are popped in order adequate to their priorities") {
                REQUIRE(queue.pop() == 2);
                REQUIRE(queue.pop() == 1);
                REQUIRE(queue.pop() == 0);
            }
        }

        WHEN("six values are added") {
            queue.emplace(Priority::normal, 1);
            queue.emplace(Priority::low, 0);
            queue.emplace(Priority::high, 2);
            queue.emplace(Priority::low, 0);
            queue.emplace(Priority::normal, 1);
            queue.emplace(Priority::high, 2);

            THEN("all values are popped in order adequate to their priorities") {
                REQUIRE(queue.pop() == 2);
                REQUIRE(queue.pop() == 2);
                REQUIRE(queue.pop() == 1);
                REQUIRE(queue.pop() == 1);
                REQUIRE(queue.pop() == 0);
                REQUIRE(queue.pop() == 0);
            }
        }
    }

    GIVEN("unsafe priority queue filled with values") {
        concurrent::unsafe_priority_queue<Priority, int> queue(
                {
                    std::make_pair(Priority::normal, 1),
                    std::make_pair(Priority::low, 0),
                    std::make_pair(Priority::high, 2),
                    std::make_pair(Priority::low, 0),
                    std::make_pair(Priority::normal, 1),
                    std::make_pair(Priority::high, 2),
                }
        );

        WHEN("nothing else happens") {
            THEN("all values are popped in order adequate to their priorities") {
                REQUIRE(queue.pop() == 2);
                REQUIRE(queue.pop() == 2);
                REQUIRE(queue.pop() == 1);
                REQUIRE(queue.pop() == 1);
                REQUIRE(queue.pop() == 0);
                REQUIRE(queue.pop() == 0);
            }
        }
    }
}

