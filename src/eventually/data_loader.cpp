
#include <eventually/data_loader.hpp>

namespace eventually {

    data_exception::data_exception(const std::string& desc):
    _description(desc)
    {
    }

    const char* data_exception::what() const throw()
    {
        return _description.c_str();
    }

}