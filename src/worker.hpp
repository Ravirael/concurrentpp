#pragma once

#include <condition_variable>
#include <atomic>
#include <thread>

namespace concurrent {
    template<class Queue, class Thread = std::thread>
    class worker {
    public:
        using queue_type = Queue;
        using thread_type = Thread;

    private:
        queue_type &m_task_queue;
        std::mutex &m_mutex;
        std::condition_variable &m_queue_not_empty;
        std::condition_variable &m_queue_empty;
        std::atomic_bool m_stopped{true};
        thread_type m_thread;

    public:
        worker(
                queue_type &task_queue,
                std::mutex &mutex,
                std::condition_variable &queue_not_empty,
                std::condition_variable &queue_empty
        ):
                m_task_queue(task_queue),
                m_mutex(mutex),
                m_queue_not_empty(queue_not_empty),
                m_queue_empty(queue_empty) {

        }

        worker(worker &&other) noexcept:
            m_task_queue(other.m_task_queue),
            m_mutex(other.m_mutex),
            m_queue_not_empty(other.m_queue_not_empty),
            m_queue_empty(other.m_queue_empty) {

            if (other.running()) {
                start();
            }
            other.stop();
        }

        void start() {
            if (!running()) {
                m_stopped = false;
                m_thread = std::thread{[this] { consume_and_execute(); }};
            }
        }

        bool running() const {
            return !m_stopped;
        }

        void stop() {
            m_stopped = true;
        }

        ~worker() {
            // fallback stopping, usually should be stopped before destructor
            if (running()) {
                stop();
                m_queue_not_empty.notify_all();
            }

            if (m_thread.joinable()) {
                m_thread.join();
            }
        }

    private:
        void consume_and_execute() {
            while (true) {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_queue_not_empty.wait(lock, [this]{ return !m_task_queue.empty() || m_stopped; });

                if (m_stopped) {
                    break;
                }

                auto task = m_task_queue.pop();

                const bool notify_empty = m_task_queue.empty();
                lock.unlock();

                if (notify_empty) {
                    m_queue_empty.notify_one();
                }

                task();
            }
        }
    };
}


