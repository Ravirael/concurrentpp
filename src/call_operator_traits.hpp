#pragma once

#include <type_traits>

namespace concurrent {
// Primary template with a static assertion
// for a meaningful error message
// if it ever gets instantiated.
// We could leave it undefined if we didn't care.

    template<typename, typename T>
    struct has_call_operator {
        static_assert(
                std::integral_constant<T, false>::value,
                "Second template parameter needs to be of function type.");
    };

// specialization that does the checking

    template<typename C, typename Ret, typename... Args>
    struct has_call_operator<C, Ret(Args...)> {
    private:
        template<typename T>
        static constexpr auto check(T *)
        -> typename
        std::is_same<
                decltype(std::declval<T>().operator()(std::declval<Args>()...)),
                Ret    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        >::type;  // attempt to call it and see if the return type is correct

        template<typename>
        static constexpr std::false_type check(...);

        typedef decltype(check<C>(0)) type;

    public:
        static constexpr bool value = type::value;
    };

}