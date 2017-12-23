cmake_minimum_required(VERSION 3.9)
project(thread_pool_tests)

set(CMAKE_CXX_STANDARD 11)

FUNCTION(PREPEND var prefix)
    SET(listVar "")
    FOREACH(f ${ARGN})
        LIST(APPEND listVar "${prefix}/${f}")
    ENDFOREACH(f)
    SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

include(${CMAKE_CURRENT_SOURCE_DIR}/../src/CMakeLists.txt)
PREPEND(ABSOLUTE_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../src ${SOURCE_FILES})
set(TEST_SOURCE_FILES main.cpp)
add_executable(thread_pool_tests ${TEST_SOURCE_FILES} ${ABSOLUTE_SOURCE_FILES})
target_link_libraries(thread_pool_tests pthread)