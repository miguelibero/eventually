
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
        mutable std::tuple<Args...> _args;

    public:

        template <typename Work, typename std::enable_if<is_callable_with_result<Work(Args...), Result>::value, int>::type = 0>
        handler(Work&& w, Args&&... args)
            : _work(std::forward<Work>(w)),
              _args(std::forward<Args>(args)...)
        {
        }

        Result operator()() const
        {
            return apply(_work, std::move(_args));
        }
    };

    template <typename Work, typename... Args>
    using work_handler = handler<typename result_of<Work(Args...)>::type, Args...>;

    template <typename Work, typename... Args>
    auto make_handler(Work&& w, Args&&... args) -> work_handler<Work, Args...>
    {
        return work_handler<Work, Args...>(std::forward<Work>(w), std::forward<Args>(args)...);
    }

}

#endif
