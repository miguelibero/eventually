#ifndef _eventually_setup_data_loader_hpp_
#define _eventually_setup_data_loader_hpp_

#include <eventually/data_loader.hpp>

namespace eventually {

	class dispatcher;
    class connection;

    template<typename Loader,
        typename std::enable_if<can_load_data<Loader>::value, int>::type = 0,
        typename std::enable_if<has_dispatcher<Loader>::value, int>::type = 0>
	class setup_data_loader
	{
    public:
        typedef std::function<void(std::string& name)> name_setup;
        typedef std::function<void(data& data)> data_setup;

    private:
        Loader* _loader; 
        bool _delete_loader;
        name_setup _name_setup;
        data_setup _data_setup;

        data_ptr when_loaded(data_ptr&& data)
        {
            if(_data_setup)
            {
                _data_setup(*data);
            }
            return std::move(data);
        }

    public:

        setup_data_loader(Loader* l=nullptr):
        _loader(l), _delete_loader(true)
        {
        }

        setup_data_loader(Loader& l):
        _loader(&l), _delete_loader(false)
        {
        }

        ~setup_data_loader()
        {
            if(_delete_loader)
            {
                delete _loader;
            }
        }

        void set_name_setup(const name_setup& setup)
        {
            _name_setup = setup;
        }

        void set_data_setup(const data_setup& setup)
        {
            _data_setup = setup;
        }

        std::future<data_ptr> load(connection& c, const std::string& name)
        {
            if(!_loader)
            {
                throw new data_exception("No loader found.");
            }
            std::string sname(name);
            if(_name_setup)
            {
                _name_setup(sname);
            }
            return _loader->get_dispatcher().when(
                std::bind(&setup_data_loader::when_loaded, this, std::placeholders::_1),
                _loader->load(c, sname));
        }

        std::future<data_ptr> load(const std::string& name)
        {
            connection conn;
            return load(conn, name);
        }
	};
}

#endif