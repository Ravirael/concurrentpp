# ConcurrentPP

This library aims to provide some functionality related with concurrent
programming missing in C++14 with strong focus on thread pools.

What it provides:

* task queue which uses fixed size thread pool
* task queue which uses dynamically sized thread pool
* synchronization barrier
* semaphore


## Build statuses

### Linux gcc 4.9+

| gcc 4.9 | gcc 5 | gcc 6 | gcc 7 |
| ------- | ----- | ----- | ----- |
| [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/Ravirael/concurrentpp/branches/master/4)](https://travis-ci.org/Ravirael/concurrentpp) | [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/Ravirael/concurrentpp/branches/master/1)](https://travis-ci.org/Ravirael/concurrentpp) | [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/Ravirael/concurrentpp/branches/master/2)](https://travis-ci.org/Ravirael/concurrentpp) | [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/Ravirael/concurrentpp/branches/master/3)](https://travis-ci.org/Ravirael/concurrentpp)

### Linux clang 3.9+

| clang 3.9 | clang 4 | clang 5 |
| --------- | ------- | ------- |
| [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/Ravirael/concurrentpp/branches/master/5)](https://travis-ci.org/Ravirael/concurrentpp) | [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/Ravirael/concurrentpp/branches/master/6)](https://travis-ci.org/Ravirael/concurrentpp) | [![Build Status](https://travis-matrix-badges.herokuapp.com/repos/Ravirael/concurrentpp/branches/master/7)](https://travis-ci.org/Ravirael/concurrentpp) |

## Installation

The library contains of multiple header files. Add contents of `src`
to your include path.

## Usage

The classes are as generic as possible, so in order to avoid long type
declarations, file `task_queues.hpp` provides most useful aliases.

N-threaded task queues types:

* `n_threaded_fifo_task_queue`
* `n_threaded_lifo_task_queue`
* `n_threaded_priority_task_queue`

Dynamic task queues types:

* `dynamic_fifo_task_queue`
* `dynamic_lifo_task_queue`
* `dynamic_priority_task_queue`

### Simple example

Using fifo and lifo queues:

```C++
    #include <task_queues.hpp>
    #include <iostream>

    int main() {
        // Declare a queue which uses 4-threaded thread pool
        n_threaded_fifo_task_queue queue(4);

        // Add task to queue using push method.
        // You can push everything convertible
        // to std::function<void(void)>.
        queue.push([] { std::cout << "Hello from task queue!"; });

        // Don't worry about joining threads and task completion -
        // all tasks are guaranteed to be finished after destruction of
        // queue.
    }
```

### Waiting for tasks completion

In some scenarios you may want to wait explicitly for task completion
but avoid destroying task queue. For this use `wait_for_tasks_completion`
method.


```C++
    #include <task_queues.hpp>
    #include <iostream>

    int main() {
        n_threaded_fifo_task_queue queue(4);

        // Add tasks to queue.
        queue.push([] { /* a task */ });

        queue.wait_for_tasks_completion();

        // Here you can be sure that all previously pushed tasks are
        // finished.
    }
```

### Getting task result

Getting a return value from task is also possible. The `std::future`
class is used to represent the result of future computations.

```C++
    #include <task_queues.hpp>
    #include <iostream>

    int main() {
        n_threaded_fifo_task_queue queue(4);

        // Add tasks to queue and wait for result.
        std::future<int> future_result = queue.push_with_result([] { return 4; });

        // Blocks until task is completed.
        int result = future_result.get();

        // Prints 4.
        std::cout << result << std::endl;
    }
```

### Using priority task queue

Usage of priority task queue is very similar to usage of other types of
queues but all tasks are required to have priority. Tasks with greater
priority are taken from queue by worker threads before tasks with
lesser priority.

```C++
    #include <task_queues.hpp>
    #include <iostream>

    int main() {
        n_threaded_priority_task_queue queue(4);

        // Use emplace method to add task with priority to queue.
        task_queue.emplace(
                            10, // Priority is 10.
                            [] {
                                // Do somehting
                            }
                    );

        // You can also push a pair of priority and task.
        task_queue.push(std::make_pair(0, []{/* Do something. */}));
    }
```