#include "stdexcept"
#include "Request.hpp"

Request::Request(const int method, const std::string host, const int port, const std::string path) : _host(host), _port(port), _path(path), _method(method) {
    if (method < 1 || method > 3)
        throw std::runtime_error("Invalid method when constructing Request");
}

Request::~Request() {}
