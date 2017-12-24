#pragma once

#include <mutex>
#include <condition_variable>

namespace concurrent {

    template <class Mutex>
    class barrier {
    public:
        using mutex_type = Mutex;

    private:
        mutex_type m_mutex;
        std::condition_variable m_cv;
        std::size_t m_count;

    public:
        explicit barrier(std::size_t count) :
                m_count{count} {

        }

        void wait() {
            std::unique_lock<mutex_type> lock{m_mutex};
            if (--m_count == 0) {
                m_cv.notify_all();
            } else {
                m_cv.wait(lock, [this] { return m_count == 0; });
            }
        }

        mutex_type &mutex() {
            return m_mutex;
        }
    };
}


