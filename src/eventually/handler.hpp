
#ifndef _eventually_handler_hpp_
#define _eventually_handler_hpp_

#include <eventually/apply.hpp>
#include <eventually/template_helper.hpp>
#include <eventually/is_callable.hpp>

#include <functional>

namespace eventually {

    /**
     * Contains a function object and its parameters
     */
    template <typename Result, typename... Args>
    class handler
    {
    private:
        std::function<Result(Args...)> _work;
        std::tuple<Args...> _args;

    public:

        template <typename Work, typename std::enable_if<is_callable_with_result<Work(Args...), Result>::value, int>::type = 0>
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
