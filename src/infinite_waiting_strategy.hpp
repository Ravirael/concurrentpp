#pragma once

#include <mutex>
#include <condition_variable>

namespace concurrent {
    class infinite_waiting_strategy {
    public:
        template < class Predicate >
        bool operator()(
                std::condition_variable &condition_variable,
                std::unique_lock<std::mutex> &lock,
                Predicate &&predicate
        ) {
            condition_variable.wait(lock, std::forward<Predicate>(predicate));
            return true;
        }
    };
}


