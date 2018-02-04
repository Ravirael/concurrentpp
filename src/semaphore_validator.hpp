#pragma once

#include <type_traits>
#include "fake_semaphore.hpp"

namespace concurrent {
    template <class Semaphore>
    struct is_semaphore_fake: std::false_type {
    };

    template <>
    struct is_semaphore_fake<fake_semaphore>: std::true_type {
    };
}


