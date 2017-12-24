#pragma once

#include <deque>
#include <mutex>

namespace concurrent {

    template <class T, class Container = std::deque<T>>
    class unsafe_fifo_queue {
    public:
        using poped_value_type = T;
        using pushed_value_type = T;
        using container_type = Container;

    private:
        container_type m_container;

    public:
        T pop() {
            const T element{std::move(m_container.front())};
            m_container.pop_front();
            return std::move(element);
        }

        void push(const T &element) {
            m_container.push_back(element);
        }

        void push(T &&element) {
            m_container.push_back(std::move(element));
        }

        template< class... Args >
        void emplace( Args&&... args ) {
            m_container.emplace_back(std::forward<Args>(args)...);
        }

        bool empty() const {
            return m_container.empty();
        }

        void clear() {
            return m_container.clear();
        }

        std::size_t size() const {
            return m_container.size();
        }
    };

}

