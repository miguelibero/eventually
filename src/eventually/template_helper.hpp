
#ifndef _eventually_template_helper_hpp_
#define _eventually_template_helper_hpp_


namespace eventually {

    namespace template_helper
    {
        template <class T1, class ...T>
        struct first
        {
            typedef T1 type;
        };

        template <class T1, class ...T>
        struct last
        {
            typedef typename last<T...>::type type;
        };

        template <class T1>
        struct last<T1>
        {
            typedef T1 type;
        };
    }

}

#endif