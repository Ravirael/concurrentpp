cmake_minimum_required(VERSION 3.9)
project(thread_pool_tests)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c++14")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -ggdb -O0 -fsanitize=thread")
set(CMAKE_CXX_FLAGS_RELSANITIZE "${CMAKE_CXX_FLAGS} -O3 -fsanitize=thread")
set(CMAKE_CXX_FLAGS_DEBUGNOSANITIZE "${CMAKE_CXX_FLAGS} -ggdb -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")



FUNCTION(PREPEND var prefix)
    SET(listVar "")
    FOREACH(f ${ARGN})
        LIST(APPEND listVar "${prefix}/${f}")
    ENDFOREACH(f)
    SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

include_directories(../src)
include(${CMAKE_CURRENT_SOURCE_DIR}/../src/CMakeLists.txt)
PREPEND(ABSOLUTE_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../src ${SOURCE_FILES})
set(TEST_SOURCE_FILES unit/main.cpp unit/worker_tests.cpp unit/spy_thread.cpp unit/spy_thread.h unit/n_threaded_fifo_task_queue_tests.cpp unit/n_threaded_priority_task_queue_tests.cpp unit/test_configuration.h ../src/barrier.hpp unit/unsafe_priority_queue_tests.cpp unit/dynamic_fifo_task_queue_tests.cpp unit/parallel_for_each_tests.cpp)
add_executable(thread_pool_tests ${TEST_SOURCE_FILES} ${ABSOLUTE_SOURCE_FILES})
target_link_libraries(thread_pool_tests pthread)

add_executable(thread_pool_performance_test performance/performance.cpp performance/lifetime_logger.h ${ABSOLUTE_SOURCE_FILES})
target_link_libraries(thread_pool_performance_test pthread curl)
