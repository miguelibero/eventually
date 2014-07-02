
#ifndef _eventually_petition_impl_hpp_
#define _eventually_petition_impl_hpp_

#include <eventually/petition.hpp>

namespace eventually {

    petition::petition():
    _data(new petition_data())
    {

    }

    void petition::cancel()
    {
        _data->cancel();
    }

    template<typename F>
    void petition::process(const F& function) const
    {
        return _data->process(function);
    }

    template<typename F>
    void petition::check(const F& function) const
    {
        return _data->check(function);
    }    

    petition_data::petition_data():
    _active(true)
    {
    }

    void petition_data::cancel()
    {
        std::lock_guard<decltype(_active_mutex)> lock(_active_mutex);
        _active = false;
    }

    template<typename F>
    void petition_data::process(const F& function) const
    {
        std::lock_guard<decltype(_active_mutex)> lock(_active_mutex);
        if(_active)
        {
            function();
        }
    }

    template<typename F>
    void petition_data::check(const F& function) const
    {
        std::lock_guard<decltype(_active_mutex)> lock(_active_mutex);
        function(_active);
    }

}

#endif