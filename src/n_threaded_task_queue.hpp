#pragma once
#include <vector>
#include <condition_variable>
#include <future>
#include <type_traits>
#include "worker.hpp"
#include "workers_pool.hpp"
#include "infinite_waiting_strategy.hpp"
#include "task_queue_base.hpp"

namespace concurrent {
    template <class Queue, class Thread>
    class n_threaded_task_queue: public task_queue_base<Queue> {
    public:
        using queue_type = Queue;
        using pushed_value_type = typename Queue::pushed_value_type;
        using thread_type = Thread;
        using worker_type = concurrent::worker<queue_type, concurrent::infinite_waiting_strategy, thread_type>;

    private:
        concurrent::workers_vector<worker_type> m_workers;

    public:
        explicit n_threaded_task_queue(
                std::size_t number_of_threads = std::thread::hardware_concurrency(),
                queue_type queue = queue_type()
        ):
            task_queue_base<Queue>(std::move(queue)),
            m_workers() {
            m_workers.reserve(number_of_threads);

            for (std::size_t i = 0u; i < number_of_threads; ++i) {
                m_workers.emplace_back(
                        this->m_task_queue,
                        this->m_queue_mutex,
                        this->m_queue_not_empty,
                        this->m_queue_empty,
                        this->m_worker_exited
                );
            }

            m_workers.start();
        }

        void push(const pushed_value_type &element) {
            {
                std::lock_guard<std::mutex> lock(this->m_queue_mutex);
                this->m_task_queue.push(element);
            }
            this->m_queue_not_empty.notify_one();
        }

        void push(pushed_value_type &&element) {
            {
                std::lock_guard<std::mutex> lock(this->m_queue_mutex);
                this->m_task_queue.push(std::move(element));
            }
            this->m_queue_not_empty.notify_one();
        }

        template< class... Args >
        void emplace( Args&&... args ) {
            {
                std::lock_guard<std::mutex> lock(this->m_queue_mutex);
                this->m_task_queue.emplace(std::forward<Args>(args)...);
            }
            this->m_queue_not_empty.notify_one();
        }

        template < class R>
        std::future<R> push_with_result(const std::function<R()> &function) {
            auto task = std::make_shared<std::packaged_task<R()>>(function);
            auto result = task->get_future();
            push([task]{task->operator()();});
            return result;
        }

        ~n_threaded_task_queue() {
            this->wait_until_is_empty();
            m_workers.stop();

            // wake all workers to be able to join their threads in destructor
            this->m_queue_not_empty.notify_all();
        }
    };
}