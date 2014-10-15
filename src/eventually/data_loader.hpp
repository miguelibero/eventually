#ifndef _eventually_data_loader_hpp_
#define _eventually_data_loader_hpp_

#include <future>
#include <exception>
#include <string>
#include <vector>
#include <cstdint>
#include <type_traits>

namespace eventually {

    class connection;
    class dispatcher;

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

#ifndef _MSC_VER

    /**
     * The type trait to detect a valid data loader.
     * Data loaders should have a method called
     * `std::future<data_ptr> load(connection& c, const std::string& name)`
     */
    template <typename T>
    using can_load_data = std::is_same<
      decltype(std::declval<T>().load(*(connection*)nullptr, std::string())), std::future<data_ptr>>;

    /**
     * The type trait to detect a class that has a dispatcher
     * These classes should have a method called
     * `dispatcher& get_dispatcher()`
     */    
    template <typename T>
    using has_dispatcher = std::is_same<
      decltype(std::declval<T>().get_dispatcher()), dispatcher&>;
#else
	template<typename T>
	struct can_load_data
	: std::true_type
	{};

	template<typename T>
	struct has_dispatcher
		: std::true_type
	{};
#endif
}

#endif