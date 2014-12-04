
#ifndef _eventually_handler_hpp_
#define _eventually_handler_hpp_

#include <eventually/apply.hpp>
#include <eventually/template_helper.hpp>
#include <eventually/is_callable.hpp>

namespace eventually {

    /**
     * Contains a function object and its parameters
     */
    template <typename... Args>
    class handler
    {
    private:
        mutable std::tuple<Args...> _args;

    public:

        handler(Args&&... args):
            _args(std::forward<Args>(args)...)
        {
        }

        template <typename Work,
            typename std::enable_if<is_callable<Work(Args&&...)>::value, int>::type = 0>
        typename result_of<Work(Args&&...)>::type operator()(Work&& w) const
        {
            return apply(w, std::move(_args));
        }

        /*
        template <typename Work,
            typename std::enable_if<is_callable<Work(Args&...)>::value, int>::type = 0>
        typename result_of<Work(Args&...)>::type operator()(Work&& w) const
        {
            return apply(w, _args);
        }
        */
    };

    template <typename... Args>
    handler<Args...> make_handler(Args&&... args)
    {
        return handler<Args...>(std::forward<Args>(args)...);
    }

}

#endif
