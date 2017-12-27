#pragma once

#include <functional>
#include "n_threaded_task_queue.hpp"
#include "unsafe_fifo_queue.hpp"
#include "dynamic_task_queue.hpp"


namespace concurrent {
    using n_threaded_fifo_task_queue = n_threaded_task_queue<
            concurrent::unsafe_fifo_queue<std::function<void()>>,
            std::thread
    >;

    using dynamic_fifo_task_queue = dynamic_task_queue<
            concurrent::unsafe_fifo_queue<std::function<void()>>,
            std::thread
    >;
}