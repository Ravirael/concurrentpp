#pragma once
#include <vector>
#include <condition_variable>
#include <future>
#include <type_traits>
#include "worker.hpp"
#include "workers_pool.hpp"
#include "infinite_waiting_strategy.hpp"
#include "task_queue_base.hpp"
#include "timeout_waiting_strategy.hpp"

namespace concurrent {
    template <class Queue, class Thread, class Duration = std::chrono::milliseconds>
    class dynamic_task_queue: public task_queue_base<Queue> {
    public:
        using queue_type = Queue;
        using pushed_value_type = typename Queue::pushed_value_type;
        using thread_type = Thread;
        using worker_type = concurrent::worker<
                queue_type,
                concurrent::infinite_waiting_strategy,
                thread_type
        >;
        using dynamic_worker_type = concurrent::worker<
                queue_type,
                concurrent::timeout_waiting_strategy<Duration>,
                thread_type
        >;

    private:
        std::mutex m_dynamic_workers_mutex;
        concurrent::workers_list<worker_type> m_core_workers;
        concurrent::workers_list<dynamic_worker_type> m_dynamic_workers;
        const std::size_t m_core_workers_size;
        const std::size_t m_dynamic_workers_max_size;
        const Duration m_timeout;
        const std::size_t m_max_queue_length;
        std::atomic_bool m_stop_cleaning{false};
        thread_type m_cleaning_thread;

    public:
        dynamic_task_queue(
                std::size_t core_pool_size = std::thread::hardware_concurrency(),
                std::size_t max_pool_size = std::thread::hardware_concurrency() * 2,
                Duration timeout = std::chrono::milliseconds(100),
                std::size_t max_queue_length = 1u,
                queue_type queue = queue_type()
        ):
                task_queue_base<Queue>(std::move(queue)),
                m_core_workers(),
                m_dynamic_workers(),
                m_core_workers_size(core_pool_size),
                m_dynamic_workers_max_size(max_pool_size - core_pool_size),
                m_timeout(std::move(timeout)),
                m_max_queue_length(max_queue_length),
                m_cleaning_thread{[this]{cleaning_thread();}} {

        }

        void push(const pushed_value_type &element) {
            {
                std::lock_guard<std::mutex> lock(this->m_queue_mutex);
                this->m_task_queue.push(element);
                if (!conditionally_increase_core_workers_size()) {
                    conditionally_increase_dynamic_workers_size();
                }
            }
            this->m_queue_not_empty.notify_one();
        }

        void push(pushed_value_type &&element) {
            {
                std::lock_guard<std::mutex> lock(this->m_queue_mutex);
                this->m_task_queue.push(std::move(element));
                if (!conditionally_increase_core_workers_size()) {
                    conditionally_increase_dynamic_workers_size();
                }
            }
            this->m_queue_not_empty.notify_one();
        }

        template< class... Args >
        void emplace( Args&&... args ) {
            {
                std::lock_guard<std::mutex> lock(this->m_queue_mutex);
                this->m_task_queue.emplace(std::forward<Args>(args)...);
                if (!conditionally_increase_core_workers_size()) {
                    conditionally_increase_dynamic_workers_size();
                }
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

        ~dynamic_task_queue() {
            this->wait_until_is_empty();

            // to close cleaning thread
            m_stop_cleaning = true;
            this->m_worker_exited.notify_one();
            m_cleaning_thread.join();

            m_core_workers.stop();
            m_dynamic_workers.stop();

            // wake all workers to be able to join their threads in destructor
            this->m_queue_not_empty.notify_all();
        }

    private:
        bool conditionally_increase_core_workers_size() {
            if (m_core_workers.size() < m_core_workers_size) {
                m_core_workers.emplace_back(
                        this->m_task_queue,
                        this->m_queue_mutex,
                        this->m_queue_not_empty,
                        this->m_queue_empty,
                        this->m_worker_exited
                );

                m_core_workers.back().start();

                return true;
            }

            return false;
        }

        bool conditionally_increase_dynamic_workers_size() {
            std::lock_guard<std::mutex> lock(m_dynamic_workers_mutex);
            if (this->m_task_queue.size() >= m_max_queue_length
                    && m_dynamic_workers.size() < m_dynamic_workers_max_size) {
                m_dynamic_workers.emplace_back(
                        this->m_task_queue,
                        this->m_queue_mutex,
                        this->m_queue_not_empty,
                        this->m_queue_empty,
                        this->m_worker_exited,
                        concurrent::timeout_waiting_strategy<Duration>(
                                m_timeout
                        )
                );
                m_dynamic_workers.back().start();
                return true;
            }

            return false;
        }

        void cleaning_thread() {
            while (true) {
                std::unique_lock<std::mutex> lock(m_dynamic_workers_mutex);
                this->m_worker_exited.wait(
                        lock,
                        [this] { return m_dynamic_workers.stopped_count() > 0u || m_stop_cleaning; }
                );

                if (m_stop_cleaning) {
                    break;
                }

                m_dynamic_workers.remove_stopped();
            }
        }
    };
}