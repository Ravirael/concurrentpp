#pragma once

#include <thread>
#include <vector>
#include <mutex>

namespace concurrent {
    class spy_thread {
        std::thread m_underlying_thread;

    public:
        static std::vector<spy_thread *> alive_threads;
        static std::mutex alive_threads_mutex;

        template <class ...Args>
        spy_thread(Args && ... args):
                m_underlying_thread(std::forward<Args>(args)...) {
            std::lock_guard<std::mutex> lock(alive_threads_mutex);
            alive_threads.push_back(this);
        }

        spy_thread &operator=(spy_thread &&other) noexcept;
        bool joinable() const;
        void join();


        ~spy_thread();
    };
}


