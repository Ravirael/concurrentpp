#pragma once

#include <type_traits>
#include <iterator>
#include <functional>

namespace concurrent {

    template <class InputIt, class TaskQueue, class UnaryOperation>
    void parallel_for_each(
            TaskQueue &taskQueue,
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
            taskQueue.emplace([copy, operation]{ operation(*copy); });
        }
        taskQueue.wait_for_finishing_tasks();
    };

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
//        taskQueue.wait_for_finishing_tasks();
//    };

}