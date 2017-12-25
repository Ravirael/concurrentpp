#pragma once
#include <vector>
#include <condition_variable>
#include <future>
#include <type_traits>
#include "worker.hpp"
#include "workers_pool.hpp"

namespace concurrent {
    template <class Queue, class Thread>
    class n_threaded_task_queue {
    public:
        using queue_type = Queue;
        using thread_type = Thread;
        using worker_type = concurrent::worker<queue_type, thread_type>;
        using pushed_value_type = typename Queue::pushed_value_type;

    private:
        queue_type m_task_queue;
        std::mutex m_queue_mutex;
        std::condition_variable m_queue_not_empty;
        std::condition_variable m_queue_empty;
        concurrent::workers_vector<worker_type> m_workers;

    public:
        explicit n_threaded_task_queue(
                std::size_t number_of_threads = std::thread::hardware_concurrency(),
                queue_type queue = queue_type()
        ):
                m_task_queue(std::move(queue)),
                m_queue_mutex(),
                m_queue_not_empty(),
                m_workers() {
            m_workers.reserve(number_of_threads);

            for (std::size_t i = 0; i < number_of_threads; ++ i) {
                m_workers.emplace_back(m_task_queue, m_queue_mutex, m_queue_not_empty, m_queue_empty);
            }

            m_workers.start();
        }

        void push(const pushed_value_type &element) {
            {
                std::lock_guard<std::mutex> lock(m_queue_mutex);
                m_task_queue.push(element);
            }
            m_queue_not_empty.notify_one();
        }

        void push(pushed_value_type &&element) {
            {
                std::lock_guard<std::mutex> lock(m_queue_mutex);
                m_task_queue.push(std::move(element));
            }
            m_queue_not_empty.notify_one();
        }

        template< class... Args >
        void emplace( Args&&... args ) {
            {
                std::lock_guard<std::mutex> lock(m_queue_mutex);
                m_task_queue.emplace(std::forward<Args>(args)...);
            }
            m_queue_not_empty.notify_one();
        }

        template < class R>
        std::future<R> push_with_result(const std::function<R()> &function) {
            auto task = std::make_shared<std::packaged_task<R()>>(function);
            auto result = task->get_future();
            push([task]{task->operator()();});
            return result;
        }

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

        ~n_threaded_task_queue() {
            wait_until_is_empty();
            m_workers.stop();

            // wake all workers to be able to join their threads in destructor
            m_queue_not_empty.notify_all();
        }
    };
}


