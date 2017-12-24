#pragma once

#include <utility>
#include <numeric>
#include <vector>
#include <list>

namespace concurrent {

    template <typename Container>
    class workers_pool {
    public:
        using worker_type = typename Container::value_type;
        using iterator = typename Container::iterator;
        using const_iterator = typename Container::const_iterator;
        using size_type = typename Container::size_type;

    private:
        Container m_container;

    public:
        template < class ...Args>
        explicit workers_pool(Args  && ... args):
            m_container(std::forward<Args>(args)...) {

        }

        iterator begin() noexcept {
            return m_container.begin();
        }

        const_iterator begin() const noexcept {
            return m_container.begin();
        }

        iterator end() noexcept {
            return m_container.end();
        }

        const_iterator end() const noexcept {
            return m_container.end();
        }

        size_type size() const noexcept {
            return m_container.size();
        }

        void reserve(size_type new_capacity) {
            m_container.reserve(new_capacity);
        }

        void clear() {
            m_container.clear();
        }

        template< class... Args >
        void emplace_back( Args&&... args ) {
            m_container.emplace_back(std::forward<Args>(args)...);
        }

        void stop() {
            for (auto &worker: *this) {
                worker.stop();
            }
        }

        void start() {
            for (auto &worker: *this) {
                worker.start();
            }
        }

    };

    template < class T >
    using workers_vector = workers_pool<std::vector<T>>;

    template < class T >
    using workers_list = workers_pool<std::list<T>>;
}