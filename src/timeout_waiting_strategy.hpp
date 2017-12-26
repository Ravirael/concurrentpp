#pragma once

#include <condition_variable>

namespace concurrent {
    template < class Rep, class Period>
    class timeout_waiting_strategy {
        const std::chrono::duration<Rep, Period> m_timeout;
        std::condition_variable &m_timeout_fired;

    public:
        timeout_waiting_strategy(
                std::chrono::duration<Rep, Period> timeout,
                std::condition_variable &timeout_fired
        ) noexcept:
                m_timeout(std::move(timeout)),
                m_timeout_fired(timeout_fired) {

        }

        template < class Predicate >
        bool operator()(
                std::condition_variable &condition_variable,
                std::unique_lock<std::mutex> &lock,
                Predicate &&predicate
        ) {
            const auto result = condition_variable.wait_for(lock, m_timeout, std::forward<Predicate>(predicate));

            if (!result) {
                m_timeout_fired.notify_one();
            }

            return result;
        }
    };
}


