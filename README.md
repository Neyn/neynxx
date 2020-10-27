# Table of Contents
- [Table of Contents](#table-of-contents)
- [Neynxx](#neynxx)
- [Build & Install](#build--install)
  - [System-Wide Installation](#system-wide-installation)
  - [CMake Subdirectory](#cmake-subdirectory)
- [Usage](#usage)
  - [Configuring](#configuring)
  - [Handling Requests](#handling-requests)
  - [Creating the Server](#creating-the-server)
  - [Running the Server](#running-the-server)
- [Contributing](#contributing)
- [License](#license)

# Neynxx
```Neynxx``` is a fast Http library written in C++. You can checkout [C](https://github.com/Neyn/cneyn) and [Python](https://github.com/Neyn/neynpy) interfaces and [JSON](https://github.com/Neyn/neyson) library too. Some of the features:

+ Fast
+ Very Easy to Use
+ No External Dependencies

Since the project is new there are some limitations:

+ ```Windows``` platform isn't supported for now.
+ Partially implements [HTTP/1.1](https://tools.ietf.org/html/rfc7230) for now.
+ Uses some new features of ```Linux``` kernel so version 4.5 and above kernel is supported for now.

# Build & Install
You have two options:
+ Building and installing the library system-wide.
+ Adding to your ```CMake``` project as a subdirectory.

You can download the latest release and extract (or you can clone the repository but the latest release is more stable).

## System-Wide Installation
You can do these in the ```neynxx``` directory:

``` shell
mkdir build && cd build
cmake -DNEYN_BUILD_TESTS=OFF -DNEYN_INSTALL_LIB=ON  -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
sudo cmake --install .
```

```sudo``` might be needed or not depending on the install destination. You can use ```CMAKE_INSTALL_PREFIX``` CMake variable to control the install destination and ```BUILD_SHARED_LIBS``` to control the type of library(shared or static).

Then you can use it with various build systems. Here is an example of ```CMake```:

``` cmake
find_package(neyn REQUIRED)
add_executable(myexec main.cpp)
target_link_libraries(myexec neyn::neyn)
```

## CMake Subdirectory
You can put the ```neynxx``` directory inside your project's directory and add it as a subdirectory. For example:

``` cmake
add_subdirectory(neyn)
add_executable(myexec main.c)
target_link_libraries(myexec ${NEYN_LIBRARIES})
target_include_directories(myexec PUBLIC ${NEYN_INCLUDE_DIRS})
```

# Usage
You can include the library like this:

``` c
#include <neyn/neyn.h>
```

The library's interface exists in ```Neyn``` namespace.

## Configuring
+ Port: port number of the server.
+ IP Version: it can be ```Neyn::Address::IPV4``` or ```Neyn::Address::IPV6```.
+ Timeout: server in milliseconds. set 0 for no timeout.
+ Limit: request size limit in bytes. set 0 for no limit.
+ Threads: number of threads. set 0 for automatic detection.
+ Address: address string of the server.

Example:

``` c++
Neyn::Config config;
config.port = 8081;
config.ipvn = Address::IPV4;
config.timeout = 0;
config.limit = 0;
config.threads = 1;
config.address = "0.0.0.0";
```

## Handling Requests
Handling of input requests is done by passing a function to the server. This function takes ```Neyn::Request``` and ```Neyn::Response``` and a user-defined data. Request struct has ```port, address, major, minor, method, path, body, header``` fields. Response struct has ```status, header, body``` fields.

Example:

``` c++
void handler(Request &request, Response &response) 
{
    response.body = "Hello"; 
}
```

## Creating the Server
You need to create a server object and pass the created configuration and handler function to it. Here is how:

``` c++
Server server(config, handler);
```

## Running the Server
You must call ```Neyn::Server::run``` function and pass the server object to it. If you want the function to be non-blocking you can pass ```false``` as the last arguement and ```true``` otherwise. You can stop a non-blocking server by calling ```Neyn::Server::kill``` on it. 

Example:

``` c++
auto error = server.run();
cout << int(error) << endl;
```

# Contributing
You can report bugs, ask questions and request features on [issues page](../../issues). Pull requests are not accepted right now.

# License
This library is licensed under BSD 3-Clause permissive license. You can read it [here](LICENSE).
