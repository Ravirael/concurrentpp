#pragma once

#include <map>

namespace concurrent {
    template <class Priority, class T, class Container = std::multimap<Priority, T, std::greater<Priority>>>
    class unsafe_priority_queue {
    public:
        using poped_value_type = typename Container::mapped_type;
        using pushed_value_type = typename Container::value_type;
        using container_type = Container;

    private:
        container_type m_container;

    public:
        explicit unsafe_priority_queue(container_type container = container_type()):
            m_container(std::move(container)) {

        }

        poped_value_type pop() {
            T element{std::move(m_container.begin()->second)};
            m_container.erase(m_container.begin());
            return std::move(element);
        }

        void push(const pushed_value_type &element) {
            m_container.insert(element);
        }

        template< class... Args >
        void emplace( Args&&... args ) {
            m_container.emplace(std::forward<Args>(args)...);
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


