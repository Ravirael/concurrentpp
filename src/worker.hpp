#pragma once

#include <condition_variable>
#include <atomic>
#include <thread>

namespace concurrent {
    template<class Queue, class Mutex>
    class worker {
    public:
        using mutex_type = Mutex;
        using queue_type = Queue;

    private:
        queue_type &m_task_queue;
        mutex_type &m_mutex;
        std::condition_variable &m_queue_not_empty;
        std::condition_variable &m_queue_empty;
        std::atomic_bool m_stopped{true};
        std::atomic_bool m_executing_task{false};
        std::thread m_thread;

    public:
        worker(
                queue_type &task_queue,
                mutex_type &mutex,
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

        bool executing_task() const {
            return m_executing_task;
        }

        bool running() const {
            return !m_stopped;
        }

        void stop() {
            m_stopped = true;
        }

        ~worker() {
            if (m_thread.joinable()) {
                m_thread.join();
            }
        }

    private:
        void consume_and_execute() {
            std::unique_lock<mutex_type> lock(m_mutex, std::defer_lock);
            while (true) {
                lock.lock();
                m_queue_not_empty.wait(lock, [this]{ return !m_task_queue.empty() || m_stopped; });

                if (m_stopped) {
                    break;
                }

                auto task = m_task_queue.pop();

                if (m_task_queue.empty()) {
                    m_queue_empty.notify_one();
                }

                lock.unlock();
                m_executing_task = true;
                task();
                m_executing_task = false;
            }
        }
    };
}


