
#include <eventually/connection.hpp>

namespace eventually {

    const char* connection_interrupted::what() const throw()
    {
        return "connection interrupted";    
    }

    connection::connection():
    _data(std::make_shared<connection_data>())
    {
    }

    connection::~connection()
    {
    }

    void connection::interrupt() noexcept
    {
        _data->interrupt_flag = true;
    }

    void connection::interruption_point()
    {
        if(_data->interrupt_flag)
        {
            throw connection_interrupted();
        }
    }

    scoped_connection::~scoped_connection()
    {
        interrupt();
    }

}
