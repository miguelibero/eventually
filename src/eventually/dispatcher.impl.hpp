
#ifndef _eventually_dispatcher_impl_hpp_
#define _eventually_dispatcher_impl_hpp_

#include <eventually/dispatcher.hpp>
#include <cassert>

namespace eventually {

    template<typename F> 
    petition dispatcher<F>::dispatch(const F& function)
    {
        auto e = std::make_pair(petition(), function);
        std::lock_guard<decltype(_petitions_mutex)> lock(_petitions_mutex);
        _petitions.insert(_petitions.end(), e);
        return e.first;
    }

    template<typename F> 
    void dispatcher<F>::process()
    {
        decltype(_petitions) petitions;
        {
            std::lock_guard<decltype(_petitions_mutex)> lock(_petitions_mutex);
            petitions.swap(_petitions);
        }
        for(auto& e : petitions)
        {
            if(e.first.active())
            {
                e.second();
            }
        }
    }

}

#endif