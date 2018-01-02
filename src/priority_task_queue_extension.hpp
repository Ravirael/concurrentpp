#pragma once


#pragma once
#include <memory>
#include <future>

namespace concurrent {
    template < class TaskQueue >
    class priority_task_queue_extension: public TaskQueue {
    public:
        using TaskQueue::TaskQueue;

        template <
                class P,
                class F,
                typename R = decltype(std::declval<F>()())
        >
        std::future<R> push_with_result(std::pair<P, F> pair) {
            auto task = std::make_shared<std::packaged_task<R()>>(std::move(pair.second));
            auto result = task->get_future();
            this->push(std::make_pair(std::move(pair.first), [task]{task->operator()();}));
            return result;
        }

        template <
                class P,
                class F,
                typename R = decltype(std::declval<F>()())
        >
        std::future<R> emplace_with_result(P &&priority, F &&function) {
            auto task = std::make_shared<std::packaged_task<R()>>(function);
            auto result = task->get_future();
            this->emplace(std::forward<P>(priority), [task]{task->operator()();});
            return result;
        }
    };
}


