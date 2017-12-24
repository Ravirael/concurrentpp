cmake_minimum_required(VERSION 3.9)
project(thread_pool VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES worker.hpp unsafe_fifo_queue.hpp n_threaded_task_queue.hpp task_queues.hpp workers_pool.hpp unsafe_priority_queue.hpp)