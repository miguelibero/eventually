
#ifndef _eventually_handler_hpp_
#define _eventually_handler_hpp_

#include <functional>
#include <eventually/apply.hpp>

namespace eventually {

    namespace handler_helper
    {
        template <std::size_t... Ts>
        struct index {};
        
        template <std::size_t N, std::size_t... Ts>
        struct gen_seq : gen_seq<N - 1, N - 1, Ts...> {};
        
        template <std::size_t... Ts>
        struct gen_seq<0, Ts...> : index<Ts...> {};
    }

    /**
     * Contains a function object and its parameters
     */
    template <class Result, class... Args>
    class handler
    {
    private:
        std::function<Result(Args...)> _work;
        std::tuple<Args...> _args;

    public:

        template <class Work>
        handler(Work&& w, Args&&... args)
            : _work(std::forward<Work>(w)),
              _args(std::forward<Args>(args)...)
        {
        }

        Result operator()()
        {
            return apply(_work, _args);
        }
    };

    template <typename Work, typename... Args>
    auto make_handler(Work&& w, Args&&... args) -> handler<decltype(w(args...)), Args...>
    {
        return handler<decltype(w(args...)), Args...>(std::forward<Work>(w), std::forward<Args>(args)...);
    }

}

#endif