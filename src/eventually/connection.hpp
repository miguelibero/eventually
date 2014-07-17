
#ifndef _eventually_connection_hpp_
#define _eventually_connection_hpp_

#include <memory>
#include <exception>

namespace eventually {

    struct connection_data;

    class connection_interrupted : public std::exception
    {
        virtual const char* what() const throw();
    };

    class connection
    {
    private:
        std::shared_ptr<connection_data> _data;
    public:
        connection();
        virtual ~connection();
        void interrupt() noexcept;
        void interruption_point();
    };

    class scoped_connection : public connection
    {
    public:
        virtual ~scoped_connection();
    };

}

#endif