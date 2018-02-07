#pragma once

#include <type_traits>
#include <iterator>
#include <functional>
#include "call_operator_traits.hpp"

namespace concurrent {

    template <class InputIt, class TaskQueue, class UnaryOperation>
    void parallel_for_each(
            TaskQueue &task_queue,
            InputIt begin,
            InputIt end,
            UnaryOperation operation,
            std::enable_if_t<
                    std::is_same<
                            std::function<void(void)>,
                            typename TaskQueue::pushed_value_type
                    >::value
            >* = nullptr
    ) {
        for (; begin != end; ++begin) {
            auto copy(begin);
            task_queue.emplace([copy, operation]{ operation(*copy); });
        }
        task_queue.wait_for_tasks_completion();
    }

    template <
            class InputIt,
            class TaskQueue,
            class TaskConstructor
    >
    void parallel_for_each_construct(
            TaskQueue &task_queue,
            InputIt begin,
            InputIt end,
            TaskConstructor operation
    ) {
        for (; begin != end; ++begin) {
            task_queue.emplace(operation(*begin));
        }
        task_queue.wait_for_tasks_completion();
    }

//    template <class InputIt, class TaskQueue, class UnaryOperation>
//    void parallel_for_each(
//            TaskQueue &taskQueue,
//            InputIt begin,
//            InputIt end,
//            UnaryOperation operation,
//            std::enable_if_t<
//                    std::is_convertible<
//                            decltype(std::declval<UnaryOperation>()(typename std::iterator_traits<InputIt>::reference)),
//                            typename TaskQueue::pushed_value_type
//                    >::value
//            >* = nullptr
//    ) {
//        for (; begin != end; ++begin) {
//            taskQueue.emplace(operation(*begin));
//        }
//        taskQueue.wait_for_tasks_completion();
//    };

}