#pragma once

#include <mutex>
#include <condition_variable>

namespace concurrent {
    class semaphore {
        unsigned m_counter;
        std::mutex m_mutex;
        std::condition_variable m_cv;

    public:
        explicit semaphore(unsigned initial_value):
                m_counter(initial_value) {

        }

        void acquire() {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this]{return m_counter > 0;});
            --m_counter;
        }

        void acquire(unsigned n) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this, n]{return m_counter >= n;});
            m_counter -= n;
        }

        template<typename _Rep, typename _Period>
        bool try_acquire_for(const std::chrono::duration<_Rep, _Period> &time) {
            std::unique_lock<std::mutex> lock(m_mutex);

            if (m_cv.wait_for(lock, time, [this]{return m_counter > 0;})) {
                --m_counter;
                return true;
            }

            return false;
        }

        void release() {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                ++m_counter;
            }
            m_cv.notify_one();
        }

        void release(unsigned n) {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_counter += n;
            }
            m_cv.notify_all();
        }
    };
}


