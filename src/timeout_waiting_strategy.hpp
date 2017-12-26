#pragma once

#include <condition_variable>

namespace concurrent {
    template < class Duration >
    class timeout_waiting_strategy {
        const Duration m_timeout;

    public:
        explicit timeout_waiting_strategy(
                Duration timeout
        ) noexcept:
                m_timeout(std::move(timeout)) {

        }

        template < class Predicate >
        bool operator()(
                std::condition_variable &condition_variable,
                std::unique_lock<std::mutex> &lock,
                Predicate &&predicate
        ) const {
            return condition_variable.wait_for(lock, m_timeout, std::forward<Predicate>(predicate));
        }
    };
}


