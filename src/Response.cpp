#include "Response.hpp"

Response::Response() : _responsecode(RES_INVALID) {}

Response::Response(const std::string filepath) : _responsecode(RES_FILE), _path(filepath) {}

Response::Response(const int code, const std::string filepath) : _responsecode(code), _path(filepath) {}

Response::Response(const int code, const std::string filepath, const std::string cgipath) : _responsecode(code), _path(filepath), _cgi_path(cgipath) {}

Response::~Response() {}

const int &Response::getType() const { return _responsecode; }

const std::string &Response::getPath() const { return _path; }

const std::string &Response::getCGIPath() const { return _cgi_path; }

Response &Response::operator=(const Response assign) {
	if (this == &assign)
		return *this;
	_path = assign._path;
	_cgi_path = assign._cgi_path;
	_responsecode = assign._responsecode;
	return *this;
}