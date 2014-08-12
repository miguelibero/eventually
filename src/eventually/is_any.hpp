
#ifndef _eventually_is_any_hpp_
#define _eventually_is_any_hpp_

#include <type_traits>

namespace eventually {

    /**
     * taken from this thread
     * http://stackoverflow.com/questions/17032310/how-to-make-a-variadic-is-same
     */
    template<typename T, typename... Rest>
    struct is_any : std::false_type {};

    template<typename T, typename First>
    struct is_any<T, First> : std::is_same<T, First> {};

    template<typename T, typename First, typename... Rest>
    struct is_any<T, First, Rest...>
        : std::integral_constant<bool, std::is_same<T, First>::value || is_any<T, Rest...>::value>
    {};

}

#endif
