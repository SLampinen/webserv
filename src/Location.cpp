#include "Location.hpp"
#include "ws_functions.hpp"

Location::Location(const std::string path) : ConfigSection("location"), 
	_path(path), _get(false), _post(false), _del(false), _dir_list(false), _matched_cgi(std::pair("", "")) {}

void Location::initialize() {
	size_t idx = 0;
	if (doesLineExist("methods", idx)) {
		for (size_t i = 1; !getIndexArg(idx, i).empty(); i++) {
			if (getIndexArg(idx, i) == "GET")
				_get = true;
			else if (getIndexArg(idx, i) == "POST")
				_post = true;
			else if (getIndexArg(idx, i) == "DELETE")
				_del = true;
			else
				throw std::runtime_error("Invalid method specified in location!");
		}
	} else {
		_get = true;
	}
	if (doesLineExist("root", idx))
		_rootpath = getIndexArg(idx, 1);
	if (doesLineExist("rewrite", idx))
		_rewrite = getIndexArg(idx, 1);
	if (doesLineExist("directory_index", idx) && getIndexArg(idx, 1) == "yes")
		_dir_list = true;
	if (doesLineExist("default_index", idx))
		_index_file = getIndexArg(idx, 1);
	for (size_t first = 1, previous_idx = 0; (first && doesLineExist("cgi_extension", idx)) 
		|| (!first && doesLineExist("cgi_extension", idx, previous_idx)); previous_idx = idx, first = 0) {
		_cgi.insert(std::pair(getIndexArg(idx, 1), getIndexArg(idx, 2)));
	}
}

bool Location::requestMatch(const int method, std::string const &request_path, size_t &match_size) {
	if (methodAvailable(method) && request_path.find(_path) == 0)
		return (match_size = _path.size(), true);
	return false;
}

bool Location::requestMatch(const Request &request, std::string &filepath) {
	if (request._path.find(_path) == 0 && methodAvailable(request._method))
		return (filepath = _rootpath + request._path.substr(_path.size() - 1, std::string::npos), true);
	return false;
}

std::string Location::makeRootPath(std::string const &request_path) {
	if (!_index_file.empty() && request_path.back() == '/')
		return (_rootpath + request_path.substr(_path.size() - 1, std::string::npos) + _index_file);
	return (_rootpath + request_path.substr(_path.size() - 1, std::string::npos));
}

bool Location::checkCGI(std::string const &request_path, std::string &cgi_path) {
	for (std::pair<std::string, std::string> const pair : _cgi) {
		if (std::string(request_path.rbegin(), request_path.rend()).find(std::string(pair.first.rbegin(), pair.first.rend())) == 0)
			return (cgi_path = pair.second, _matched_cgi = pair, true);
	}
	return (_matched_cgi = std::pair("", ""), false);
}

std::string Location::getLastCGISuffix() { return _matched_cgi.first; }
std::string Location::getLastCGIPath() { return _matched_cgi.second; }
bool Location::directoryIndexAllowed() { return _dir_list; }
std::string Location::defaultIndexFile() { return _index_file; }
std::string Location::getRootPath() { return _rootpath; }

bool Location::methodAvailable(const int method) {
	if (method != REQ_GET && method != REQ_POST && method != REQ_DEL)
		throw std::logic_error("Location::methodAvailable called with invalid argument: " + std::to_string(method));
	if (_get && method == REQ_GET) return true;
	if (_post && method == REQ_POST) return true;
	if (_del && method == REQ_DEL) return true;
	return false;
}

void Location::printData() {
	std::cout << "\e[0;33mLocation.printData() : \e[0;93m" << _path << std::endl;
	std::cout << "\e[0;33mMethods: ";
	if (_get) std::cout << "GET ";
	if (_post) std::cout << "POST ";
	if (_del) std::cout << "DELETE";
	std::cout << std::endl;
	if (!_rewrite.empty()) std::cout << "Rewrite: " << _rewrite << std::endl;
	if (!_rootpath.empty()) std::cout << "Root: " << _rootpath << std::endl;
	std::cout << "Directory listing: " << std::boolalpha << _dir_list << std::endl;
	if (!_index_file.empty()) std::cout << "Dir index file: " << _index_file << std::endl;
	std::cout << "\e[0m";
}
