
#include <eventually/http_data_loader.hpp>
#include <eventually/http_client.hpp>
#include <eventually/http_request.hpp>
#include <eventually/http_response.hpp>
#include <eventually/connection.hpp>
#include <eventually/dispatcher.hpp>
#include <functional>

namespace eventually {

    http_data_loader::http_data_loader(http_client* c):
    _client(c ? c : new http_client()),
    _delete_client(true)
    {
    }

    http_data_loader::http_data_loader(http_client& c):
    _client(&c),
    _delete_client(false)
    {
    }

    http_data_loader::~http_data_loader()
    {
        if(_delete_client)
        {
            delete _client;
        }
    }

    data http_data_loader::when_sent(connection& c, http_response&& resp)
    {
        return std::move(resp.get_body());
    }

    http_client& http_data_loader::get_client()
    {
        if(!_client)
        {
            throw new data_exception("No http client found.");
        }
        return *_client;
    }

    dispatcher& http_data_loader::get_dispatcher()
    {
        if(!_client)
        {
            throw new data_exception("No http client found.");
        }
        return _client->get_dispatcher();
    }

    void http_data_loader::set_request_create(const request_create& create)
    {
        _request_create = create;
    }

    http_request http_data_loader::create_request(const std::string& name)
    {
        if(_request_create)
        {
            return _request_create(name);
        }
        else
        {
            return http_request(name);
        }
    }

    std::future<data> http_data_loader::load(const std::string& name)
    {
        connection conn;
        return load(conn, name);
    }

    std::future<data> http_data_loader::load(connection& c, const std::string& name)
    {
        if(!_client)
        {
            throw new data_exception("No http client found.");
        }
        return get_dispatcher().when(c,
            std::bind(&http_data_loader::when_sent, this, c, std::placeholders::_1),
            _client->send(c, create_request(name)));
    }

}