#pragma once

namespace concurrent {
    class fake_semaphore {

    public:
        explicit fake_semaphore(unsigned) noexcept {

        }

        void acquire(unsigned) {
        }

        void acquire() {
        }

        template<typename Duration>
        bool try_acquire_for(const Duration &, unsigned) {
            return false;
        }

        template<typename Duration>
        bool try_acquire_for(const Duration &) {
            return false;
        }

        void release(unsigned ) {
        }

        void release() {
        }
    };
}


