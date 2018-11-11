#include <functional>

namespace concurrent {

    template <typename PushedValueType>
    class task_queue {
    public:
        virtual void push(PushedValueType &&pushed_value) = 0;
        virtual void clear() = 0;
        virtual void wait_until_is_empty() = 0;
        virtual std::size_t size() const = 0;
        virtual bool empty() const = 0;
        virtual ~task_queue() noexcept = default;
    };

    using default_task_queue = task_queue<std::function<void()>>;
}