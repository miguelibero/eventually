
#ifndef _eventually_apply_hpp_
#define _eventually_apply_hpp_

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>


namespace eventually {

	template<size_t N>
	struct apply_t {
	    template<typename F, typename T, typename... A>
	    static inline auto apply(F && f, T && t, A &&... a)
	        -> decltype(apply_t<N-1>::apply(
	            ::std::forward<F>(f), ::std::forward<T>(t),
	            ::std::get<N-1>(::std::forward<T>(t)), ::std::forward<A>(a)...
	        ))
	    {
	        return apply_t<N-1>::apply(::std::forward<F>(f), ::std::forward<T>(t),
	            ::std::get<N-1>(::std::forward<T>(t)), ::std::forward<A>(a)...
	        );
	    }
	};

	template<>
	struct apply_t<0> {
	    template<typename F, typename T, typename... A>
	    static inline auto apply(F && f, T &&, A &&... a)
	        -> decltype(::std::forward<F>(f)(::std::forward<A>(a)...))
	    {
	        return ::std::forward<F>(f)(::std::forward<A>(a)...);
	    }
	};

	/**
	 * Helper method that calls a function object with a tuple as arguments
	 * taken from http://stackoverflow.com/questions/687490/how-do-i-expand-a-tuple-into-variadic-template-functions-arguments
	 */
	template<typename F, typename T>
	inline auto apply(F && f, T && t)
	    -> decltype(apply_t< ::std::tuple_size<
	        typename ::std::decay<T>::type
	    >::value>::apply(::std::forward<F>(f), ::std::forward<T>(t)))
	{
	    return apply_t< ::std::tuple_size<
	        typename ::std::decay<T>::type
	    >::value>::apply(::std::forward<F>(f), ::std::forward<T>(t));
	}
}


#endif