#ifndef _eventually_data_loader_hpp_
#define _eventually_data_loader_hpp_

#include <future>
#include <exception>
#include <string>
#include <vector>
#include <cstdint>
#include <eventually/connection.hpp>

namespace eventually {

    /**
     * The exception thrown 
     */
    class data_exception : public std::exception
    {
    private:
        std::string _description;
    public:
        data_exception(const std::string& desc);
        virtual const char* what() const throw();
    };

    typedef std::vector<uint8_t> data;
    typedef std::unique_ptr<data> data_ptr;

    /**
     * The type trait to detect a valid data loader.
     * Data loaders should have a method called
     * `std::future<data_ptr> load(connection& c, const std::string& name)`
     */    
    template <typename T>
    class can_load_data {
    private:
      typedef char Yes;
      typedef Yes No[2];
      connection _conn;

      template<typename C> static auto Test(void*)
        -> decltype(std::future<data_ptr>{std::declval<C>().load(_conn, std::string())}, Yes{});

      template<typename> static No& Test(...);

    public:
        static bool const value = sizeof(Test<T>(0)) == sizeof(Yes);
    };

}

#endif