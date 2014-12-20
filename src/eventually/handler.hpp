
#ifndef _eventually_handler_hpp_
#define _eventually_handler_hpp_

#include <eventually/apply.hpp>
#include <eventually/template_helper.hpp>
#include <eventually/is_callable.hpp>
#include <eventually/connection.hpp>
#include <future>

namespace eventually {

    /**
     * Contains a tuple parameters
     * you can pass function objects and promises to fullfill
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

        template <typename Work, typename Result,
            typename std::enable_if<is_callable_with_result<Work(Args&&...), Result>::value, int>::type = 0>
        void operator()(Work&& w, connection& c, std::promise<Result>& p) const NOEXCEPT
        {
            std::lock_guard<std::mutex> lock(c.get_mutex());
            try
            {
                c.interruption_point();
                p.set_value(apply(w, std::move(_args)));
            }
            catch(...)
            {
                p.set_exception(std::current_exception());
            }
        }

        template <typename Work,
            typename std::enable_if<is_callable_with_result<Work(Args&&...), void>::value, int>::type = 0>
        void operator()(Work&& w, connection& c, std::promise<void>& p) const NOEXCEPT
        {
            std::lock_guard<std::mutex> lock(c.get_mutex());
            try
            {
                c.interruption_point();
                apply(w, std::move(_args));
                p.set_value();
            }
            catch(...)
            {
                p.set_exception(std::current_exception());
            }                
        }


        template <typename Retry,
            typename std::enable_if<is_callable_with_result<Retry(Args&...), bool>::value, int>::type = 0>
        bool operator()(Retry&& r, connection& c) const NOEXCEPT
        {
            std::lock_guard<std::mutex> lock(c.get_mutex());
            return apply(r, _args);
        }

    };

    template <typename... Args>
    handler<Args...> make_handler(Args&&... args)
    {
        return handler<Args...>(std::forward<Args>(args)...);
    }

}

#endif
