Eventually
==========

This is a C++11 library for concurrent programming.
It aims to provide a simple way to request asyncronous tasks
that can be interrupted.


`eventually::dispatcher` is a base class to implement `std::async` like functionality.

```c++
dispatcher d;

auto future1 = d.dispatch([](int a, int b){
    return a+b;
}, 2, 3);

// this will call the dispatched lambda
d.process_one();

// will return 5.0f
auto result = future.get();
```

It has `then` support to concatenate tasks.

```c++
dispatcher d;

auto f = d.then(d.dispatch([](int a, int b){
    return a+b;
}, 2, 3), [](int c){
    return 2.0f*c ;
});

// this will call the dispatched lambdas
d.process_all();

// will return 10.0f
auto result = future.get();
```

It has `connection` support to interrupt tasks.

```c++

dispatcher d;
connection c;

d.dispatch(c, (int a, int b){
    return a+b;
}, 2, 3);

c.interrupt();

// would throw eventually::connection_interrupted
auto result = future.get();

```


