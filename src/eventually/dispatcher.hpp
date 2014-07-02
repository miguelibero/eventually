
#ifndef _eventually_dispatcher_hpp_
#define _eventually_dispatcher_hpp_

#include <eventually/petition.hpp>
#include <vector>
#include <functional>

namespace eventually {

    class petition;

    template<typename F>
    class dispatcher
    {
    private:
        std::vector<std::pair<petition,F>> _petitions;
        std::recursive_mutex _petitions_mutex;        
    public:
        inline petition dispatch(const F& function);
        inline void process();
    };

}

#include <eventually/dispatcher.impl.hpp>

#endif