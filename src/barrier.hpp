#pragma once

#include <mutex>
#include <condition_variable>

namespace concurrent {

    class barrier {
        std::mutex m_mutex;
        std::condition_variable m_cv;
        std::size_t m_count;

    public:
        explicit barrier(std::size_t count) :
                m_count{count} {

        }

        barrier(const barrier &second):
            m_count(second.m_count) {

        }

        void wait() {
            std::unique_lock<std::mutex> lock{m_mutex};
            if (m_count > 0) {
                --m_count;
            }

            if (m_count == 0) {
                m_cv.notify_all();
            } else {
                m_cv.wait(lock, [this] { return m_count == 0; });
            }
        }

        template<typename _Rep, typename _Period>
        bool wait_for(const std::chrono::duration<_Rep, _Period> &time) {
            std::unique_lock<std::mutex> lock{m_mutex};
            if (m_count > 0) {
                --m_count;
            }

            if (m_count == 0) {
                m_cv.notify_all();
                return true;
            } else {
                return m_cv.wait_for(lock, time, [this] { return m_count == 0; });
            }
        }
    };
}


