
#ifndef _eventually_is_callable_hpp_
#define _eventually_is_callable_hpp_

#include <type_traits>

namespace eventually {

    /**
     * is_callable implemtnation taken from this article
     * http://talesofcpp.fusionfenix.com/post-11/true-story-call-me-maybe
     */
    namespace detail {

        template <typename T>
        using always_void = void;

        template <typename Expr, std::size_t Step = 0, typename Enable = void>
        struct is_callable_impl
        : is_callable_impl<Expr, Step + 1>
        {};

        // (t1.*f)(t2, ..., tN) when f is a pointer to a member function of a class T
        // and t1 is an object of type T or a reference to an object of type T or a
        // reference to an object of a type derived from T;
        template <typename F, typename T, typename ...Args>
        struct is_callable_impl<F(T, Args...), 0,
        always_void<decltype(
          (std::declval<T>().*std::declval<F>())(std::declval<Args>()...)
        )>
        > : std::true_type
        {};

        // ((*t1).*f)(t2, ..., tN) when f is a pointer to a member function of a class T
        // and t1 is not one of the types described in the previous item;
        template <typename F, typename T, typename ...Args>
        struct is_callable_impl<F(T, Args...), 1,
        always_void<decltype(
          ((*std::declval<T>()).*std::declval<F>())(std::declval<Args>()...)
        )>
        > : std::true_type
        {};

        // t1.*f when N == 1 and f is a pointer to member data of a class T and t1 is an
        // object of type T or a reference to an object of type T or a reference to an
        // object of a type derived from T;
        template <typename F, typename T>
        struct is_callable_impl<F(T), 2,
        always_void<decltype(
          std::declval<T>().*std::declval<F>()
        )>
        > : std::true_type
        {};

        // (*t1).*f when N == 1 and f is a pointer to member data of a class T and t1 is
        // not one of the types described in the previous item;
        template <typename F, typename T>
        struct is_callable_impl<F(T), 3,
        always_void<decltype(
          (*std::declval<T>()).*std::declval<F>()
        )>
        > : std::true_type
        {};

        // f(t1, t2, ..., tN) in all other cases.
        template <typename F, typename ...Args>
        struct is_callable_impl<F(Args...), 4,
        always_void<decltype(
          std::declval<F>()(std::declval<Args>()...)
        )>
        > : std::true_type
        {};

        template <typename Expr>
        struct is_callable_impl<Expr, 5>
        : std::false_type
        {};

    }

    template <typename Expr>
    struct is_callable
    : detail::is_callable_impl<Expr>
    {};

    template<typename Expr>
    class result_of;

    template<typename F, typename... Args>
    struct result_of<F(Args...)>
    {
      typedef decltype(std::declval<F>()(std::declval<Args>()...)) type;
    };

    template <typename Expr, typename Result>
    struct is_callable_with_result
    : std::is_same<typename result_of<Expr>::type, Result>
    {};
 
}

#endif
