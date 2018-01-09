cmake_minimum_required(VERSION 3.9)
project(thread_pool VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 14)

set(
        SOURCE_FILES
        barrier.hpp
        dynamic_task_queue.hpp
        infinite_waiting_strategy.hpp
        n_threaded_task_queue.hpp
        parallel_for_each.hpp
        priority_task_queue_extension.hpp
        semaphore.hpp
        task_queue_base.hpp
        task_queue_extension.hpp
        task_queues.hpp
        timeout_waiting_strategy.hpp
        unsafe_fifo_queue.hpp
        unsafe_lifo_queue.hpp
        unsafe_priority_queue.hpp
        worker.hpp
        workers_pool.hpp
)