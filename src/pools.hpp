#pragma once

#include <functional>
#include "fixed_size_thread_pool.hpp"
#include "unsafe_fifo_queue.hpp"

namespace concurrent {
    using fifo_fixed_size_thread_pool = fixed_size_thread_pool<
            std::mutex,
            concurrent::unsafe_fifo_queue<std::function<void()>>
    >;
}