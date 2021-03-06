Eventually
==========

This is a C++11 library for concurrent programming.
It aims to provide a simple way to request asyncronous tasks
that can be interrupted.

`eventually::dispatcher` is a base class to implement [`std::async`](http://es.cppreference.com/w/cpp/thread/async) like functionality.

```c++
dispatcher d;

std::future<int> f = d.dispatch([](int a, int b){
    return a+b;
}, 2, 3);

// this will call the dispatched lambda
d.process_one();

// will return 5
auto result = f.get();
```

It has `when` support to concatenate tasks.

```c++
dispatcher d;

auto f = d.when([](int c){
    // will be executed with the result of a+b
    return 2.0f*c ;
}, d.dispatch([](int a, int b){
    return a+b;
}, 2, 3));

// this will call the dispatched lambdas
d.process_all();

// will return 10.0f
auto result = f.get();
```

It has `connection` support to interrupt tasks.

```c++
dispatcher d;
connection c;

auto f = d.dispatch(c, (int a, int b){
    return a+b;
}, 2, 3);

c.interrupt();
d.process_one();

// would throw eventually::connection_interrupted
auto result = f.get();
```

It has `when_throw` support to capture exceptions in tasks.

```c++
dispatcher d;
bool thrown = false;

auto f = d.when_throw([&thrown](const std::exception& e){
    thrown = true;
}, d.dispatch([](int a, int b){
    throw std::exception();
    return a+b;
}, 2, 3));

d.process_one();
d.process_one();

// here thrown is already true

// would throw exception
auto result = f.get();
```


It has `when_throw_continue` support to capture exceptions in tasks
and return a value to the future.

```c++
dispatcher d;

auto f = d.when_throw_continue([](const std::exception& e){
    return 1;
}, d.dispatch([](int a, int b){
    throw std::exception();
    return a+b;
}, 2, 3));

d.process_one();
d.process_one();

// would return 1
auto result = f.get();
```

It has `when_all` support to wait for a list of tasks.

```c++
dispatcher d;

auto f = d.when_all([](int a, float b){
    // will be executed when both tasks are done
    return a*b;
}, d.dispatch([](int a, int b){
    return a+b;
}, 2, 3), d.dispatch([](float a, float b){
    return a/b;
}, 4.0f, 2.0f));

d.process_all();
// will return 10.0f
auto result = f.get();
```

`when_all` also works without callback returning a `std::tuple`.

```c++
dispatcher d;

auto f = d.when_all(d.dispatch([](int a, int b){
    return a+b;
}, 2, 3), d.dispatch([](float a, float b){
    return a/b;
}, 4.0f, 2.0f));

d.process_all();
// will return a tuple of (5, 2.0f)
auto result = f.get();
```

It has `when_any` support to wait for the first task that finishes from a list.

```c++
dispatcher d;

auto f = d.when_any([](int a){
    return a;
}, d.dispatch([](int a, int b){
    return a+b;
}, 2, 3), d.dispatch([](int a, int b){
    return a-b;
}, 3, 2));

d.process_all();
// will return 1
auto result = f.get();
```

It has `when_every` support to to call a callback on every task of a list.

```c++
dispatcher d;

auto f = d.when_every([](std::vector<int>& a){
    // called with {1} and then with {1, 5}
    for(auto& i : a)
    {
        i += 1;
    }
}, d.dispatch([](int a, int b){
    return a+b;
}, 2, 3), d.dispatch([](int a, int b){
    return a-b;
}, 3, 2));

d.process_all();
// will return a vector with {3, 6}
auto result = f.get();
```

`eventually::thread_dispatcher` processes the tasks in a finite amount of threads
(by default `std::thread::hardware_concurrency()`).

## http client

The library implements a simple http client using [libcurl](http://curl.haxx.se/libcurl/),
that takes a `eventually::dispatcher` to do the asyncronous http requests.

```c++
http_client client;
http_request req("http://httpbin.org/headers");
req.get_headers().push_back({"X-Eventually", "true"});
auto f = client.send(req);
auto resp = f.get();
```

## data loaders

The library implements some classes to load data asyncronously. The data
is stored in a vector `std::unique_ptr` and we use move semantics to
prevent from copying it.

```c++
file_data_loader loader;
connection conn;
loader.get_dispatcher().when(conn, [](std::vector<uint8_t>&& data){
    // do stuff with the file contents
}, loader.load(conn, "/etc/magic"));
```

## Usage example

This example shows a widget class that wants to get an http response
and process it in the main thread.

```c++
class widget
{
private:
    eventually::scoped_connection _http_conn;
    http_client& _http_client;
    eventually::dispatcher& _main_dispatcher;

    void on_http_response(const http_response& resp)
    {
        // process response
    }

public:

    widget(http_client& http_client, eventually::dispatcher& main_dispatcher):
    _http_client(http_client), _main_dispatcher(main_dispatcher)
    {
    }

    void init()
    {
        http_request req("http://eventually.io/test.png");
        _main_dispatcher.when(_http_conn,
            std::bind(&widget::on_http_response, this, std::placeholders::_1),
            _http_client.send(_http_conn, req));
    }
};

http_client c;
// this dispatcher should be processed in the main thread
dispatcher m;

{
    widget w(c, m);
    w.init();
}

```

## Acknowledgements

* Herb Sutter for his [concurrency talk](http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Herb-Sutter-Concurrency-and-Parallelism)
* Anthony Williams for his [C++ Concurrency In Action](http://www.cplusplusconcurrencyinaction.com/) book
