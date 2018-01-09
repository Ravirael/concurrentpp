#pragma once

#include <mutex>
#include <condition_variable>

namespace concurrent {

    template <class Queue, class Semaphore>
    class task_queue_base {
    public:
        using queue_type = Queue;
        using pushed_value_type = typename Queue::pushed_value_type;
        using semaphore_type = Semaphore;

    protected:
        queue_type m_task_queue;
        std::mutex m_queue_mutex;
        std::condition_variable m_queue_not_empty;
        std::condition_variable m_queue_empty;
        std::condition_variable m_worker_exited;
        semaphore_type m_semaphore;

        explicit task_queue_base(
                queue_type queue = queue_type()
        ):
            m_task_queue(std::move(queue)),
            m_queue_mutex(),
            m_queue_not_empty(),
            m_queue_empty(),
            m_worker_exited(),
            m_semaphore(0) {

        }

        ~task_queue_base() noexcept = default;

    public:
        void wait_until_is_empty() {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            m_queue_empty.wait(lock, [this]{ return m_task_queue.empty(); });
        }

        void clear() {
            std::lock_guard<std::mutex> lock(m_queue_mutex);
            m_task_queue.clear();
        }

        std::size_t size() {
            std::lock_guard<std::mutex> lock(m_queue_mutex);
            return m_task_queue.size();
        }

        bool empty() {
            std::lock_guard<std::mutex> lock(m_queue_mutex);
            return m_task_queue.empty();
        }
    };
}