#include "Response.hpp"

Response::Response() : _responsecode(RES_INVALID) {} // invalid, only used for placeholder

Response::Response(const std::string filepath) : _responsecode(RES_FILE), _path(filepath) {}

Response::Response(const int code, const std::string filepath) : _responsecode(code), _path(filepath) {}

Response::Response(const int code, const std::string filepath, const std::string cgipath) : _responsecode(code), _path(filepath), _cgi_path(cgipath) {}

Response::~Response() {}

const int &Response::getType() { return _responsecode; }

const std::string &Response::getPath() { return _path; }

const std::string &Response::getCGIPath() { return _cgi_path; }

Response &Response::operator=(const Response assign) {
	if (this == &assign)
		return *this;
	_path = assign._path;
	_cgi_path = assign._cgi_path;
	_responsecode = assign._responsecode;
	return *this;
}