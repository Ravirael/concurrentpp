#pragma once
#include <vector>
#include <condition_variable>
#include "worker.hpp"
#include "workers_pool.hpp"

namespace concurrent {
    template <class Mutex, class Queue>
    class fixed_size_thread_pool {
    public:
        using mutex_type = Mutex;
        using queue_type = Queue;
        using worker_type = concurrent::worker<queue_type, mutex_type>;
        using pushed_value_type = typename Queue::pushed_value_type;

    private:
        queue_type m_task_queue;
        mutex_type m_queue_mutex;
        std::condition_variable m_queue_not_empty;
        std::condition_variable m_queue_empty;
        concurrent::workers_vector<worker_type> m_workers;

    public:
        explicit fixed_size_thread_pool(std::size_t pool_size = std::thread::hardware_concurrency()):
                m_task_queue(),
                m_queue_mutex(),
                m_queue_not_empty(),
                m_workers() {
            m_workers.reserve(pool_size);

            for (std::size_t i = 0; i < pool_size; ++ i) {
                m_workers.emplace_back(m_task_queue, m_queue_mutex, m_queue_not_empty, m_queue_empty);
            }

            m_workers.start();
        }

        void push(const pushed_value_type &element) {
            {
                std::lock_guard<mutex_type> lock(m_queue_mutex);
                m_task_queue.push(element);
            }
            m_queue_not_empty.notify_one();
        }

        void push(pushed_value_type &&element) {
            {
                std::lock_guard<mutex_type> lock(m_queue_mutex);
                m_task_queue.push(std::move(element));
            }
            m_queue_not_empty.notify_one();
        }

        template< class... Args >
        void emplace( Args&&... args ) {
            {
                std::lock_guard<mutex_type> lock(m_queue_mutex);
                m_task_queue.emplace(std::forward<Args>(args)...);
            }
            m_queue_not_empty.notify_one();
        }

        void wait_until_task_queue_is_empty() {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            m_queue_empty.wait(lock, [this]{ return m_task_queue.empty(); });
        }

        void clear_tasks() {
            std::lock_guard<mutex_type> lock(m_queue_mutex);
            m_task_queue.clear();
        }

        std::size_t currently_executed_tasks() const {
            return m_workers.currently_executed_tasks();
        }

        ~fixed_size_thread_pool() {
            wait_until_task_queue_is_empty();
            m_workers.stop();

            // wake all workers to be able to join their threads in destructor
            m_queue_not_empty.notify_all();
        }
    };
}


