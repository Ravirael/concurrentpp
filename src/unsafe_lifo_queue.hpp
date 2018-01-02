#pragma once

#include <vector>

namespace concurrent {

    template <class T, class Container = std::vector<T>>
    class unsafe_lifo_queue {
    public:
        using poped_value_type = T;
        using pushed_value_type = T;
        using container_type = Container;

    private:
        container_type m_container;

    public:
        explicit unsafe_lifo_queue(container_type container = container_type()):
            m_container(std::move(container)) {

        }

        T pop() {
            T element{std::move(m_container.back())};
            m_container.pop_back();
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

