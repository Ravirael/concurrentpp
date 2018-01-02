#pragma once

#include <functional>
#include "n_threaded_task_queue.hpp"
#include "unsafe_fifo_queue.hpp"
#include "dynamic_task_queue.hpp"
#include "task_queue_extension.hpp"
#include "priority_task_queue_extension.hpp"
#include "unsafe_priority_queue.hpp"
#include "unsafe_lifo_queue.hpp"


namespace concurrent {
    using n_threaded_fifo_task_queue = task_queue_extension<
            n_threaded_task_queue<
                concurrent::unsafe_fifo_queue<std::function<void()>>,
                std::thread
        >
    >;

    using dynamic_fifo_task_queue = task_queue_extension<
            dynamic_task_queue<
                concurrent::unsafe_fifo_queue<std::function<void()>>,
                std::thread
        >
    >;

    using n_threaded_lifo_task_queue = task_queue_extension<
            n_threaded_task_queue<
                    concurrent::unsafe_lifo_queue<std::function<void()>>,
                    std::thread
            >
    >;

    using dynamic_lifo_task_queue = task_queue_extension<
            dynamic_task_queue<
                    concurrent::unsafe_lifo_queue<std::function<void()>>,
                    std::thread
            >
    >;

    using n_threaded_priority_task_queue = priority_task_queue_extension<
            n_threaded_task_queue<
                    concurrent::unsafe_priority_queue<int, std::function<void()>>,
                    std::thread
            >
    >;

    using dynamic_priority_task_queue = priority_task_queue_extension<
            dynamic_task_queue<
                    concurrent::unsafe_priority_queue<int, std::function<void()>>,
                    std::thread
            >
    >;
}