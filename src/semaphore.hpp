#pragma once

#include <mutex>
#include <condition_variable>

namespace concurrent {
    class semaphore {
        unsigned m_counter;
        std::mutex m_mutex;
        std::condition_variable m_cv;

    public:
        explicit semaphore(unsigned initial_value) noexcept:
                m_counter(initial_value) {

        }

        void acquire(unsigned n) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this, n]{return m_counter >= n;});
            m_counter -= n;
        }

        void acquire() {
            acquire(1u);
        }

        template<typename Duration>
        bool try_acquire_for(const Duration &duration, unsigned n) {
            std::unique_lock<std::mutex> lock(m_mutex);

            if (m_cv.wait_for(lock, duration, [this, n]{return m_counter >= n;})) {
                m_counter -= n;
                return true;
            }

            return false;
        }

        template<typename Duration>
        bool try_acquire_for(const Duration &duration) {
            return try_acquire_for(duration, 1u);
        }


        void release(unsigned n) {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_counter += n;
            }
            m_cv.notify_all();
        }

        void release() {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                ++m_counter;
            }
            m_cv.notify_one();
        }
    };
}


