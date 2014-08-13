
#ifndef _eventually_is_same_hpp_
#define _eventually_is_same_hpp_

#include <type_traits>

namespace eventually {

    /**
     * taken from this thread
     * http://stackoverflow.com/questions/17032310/how-to-make-a-variadic-is-same
     */
    template<typename T, typename... Rest>
    struct is_same : std::false_type {};

    template<typename T, typename First>
    struct is_same<T, First> : std::is_same<T, First> {};

    template<typename T, typename First, typename... Rest>
    struct is_same<T, First, Rest...>
        : std::integral_constant<bool, std::is_same<T, First>::value && is_same<T, Rest...>::value>
    {};

}

#endif
