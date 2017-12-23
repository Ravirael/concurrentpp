#include <iostream>
#include "../src/pools.hpp"
#include <chrono>
#include <set>



int main() {
    concurrent::fifo_fixed_size_thread_pool pool(1000);
    std::atomic_int atomic{0};
    std::cout << "Main: " << std::this_thread::get_id() << std::endl;


    for (int i = 0; i < 99; ++ i) {
        pool.push(
                [&atomic] {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    ++atomic;
                }
        );
    }
    std::cout << "Calculated value: " << atomic.load() << std::endl;
    pool.wait_until_task_queue_is_empty();
    std::cout << "Calculated value: " << atomic.load();
    return 0;
}