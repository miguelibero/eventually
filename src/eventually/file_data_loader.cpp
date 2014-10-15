
#include <eventually/file_data_loader.hpp>
#include <eventually/dispatcher.hpp>
#include <eventually/connection.hpp>
#include <eventually/thread_dispatcher.hpp>
#include <functional>
#include <cstdio> 

namespace eventually {

    file_data_loader::file_data_loader(dispatcher* d):
    _dispatcher(d ? d : new thread_dispatcher()),
    _delete_dispatcher(true)
    {
    }

    file_data_loader::file_data_loader(dispatcher& d):
    _dispatcher(&d),
    _delete_dispatcher(false)
    {
    }

    file_data_loader::~file_data_loader()
    {
        if(_delete_dispatcher)
        {
            delete _dispatcher;
        }
    }

    data_ptr file_data_loader::load_dispatched(connection& c, const std::string& name)
    {
    	data_ptr d(new data());
        size_t s = 0;

		FILE *f = nullptr;
#ifdef _MSC_VER
		if(fopen_s(&f, name.c_str(), "rb") != 0)
		{
			f = nullptr;
		}
#else
        f = fopen(name.c_str(), "rb");
#endif
		if (f == nullptr)
        { 
            throw data_exception("Could not open file.");
        } 
        fseek(f, 0, SEEK_END);
        s = ftell(f);
        fseek(f, 0, SEEK_SET);
        d->resize(s);
        if (s != fread(d->data(), 1, s, f)) 
        { 
            fclose(f);
            throw data_exception("Could not read file.");
        } 
        fclose(f);
    	return d;
    }

    dispatcher& file_data_loader::get_dispatcher()
    {
    	return *_dispatcher;
    }

    std::future<data_ptr> file_data_loader::load(const std::string& name)
    {
        connection conn;
        return load(conn, name);
    }

    std::future<data_ptr> file_data_loader::load(connection& c, const std::string& name)
    {
        if(!_dispatcher)
        {
            throw new data_exception("No dispatcher found.");
        }
        return _dispatcher->dispatch(c, std::bind(&file_data_loader::load_dispatched, c, name));
    }

}