#ifndef WS_LOCATION_HPP
# define WS_LOCATION_HPP

# include <string>
# include <stdexcept> // GCC wants this for std::runtime_error
# include <map>
# include "ConfigSection.hpp"
# include "Request.hpp"
# include <iostream> //debug

// location /
//		http_methods GET POST DEL
//		http_redirection
//		root
//		directory_list y/n
//		default_index
//		request_method
//		cgi_extension .php  .py { }

class Location : public ConfigSection {
	public:
		Location(const std::string path);

		void initialize();
		bool requestMatch(const int method, std::string const &request_path, size_t &match_size);
		bool requestMatch(const Request &request, std::string &filepath);
		std::string makeRootPath(std::string const &request_path);
		bool checkCGI(std::string const &request_path, std::string &cgi_path);
		
		const std::string _path;

	private:
		bool methodAvailable(const int method);
		bool _get, _post, _del;
		std::string _rewrite;
		std::string _rootpath;
		bool _dir_list;
		std::string _index_file;
		std::map<std::string, std::string> _cgi;

	public: //debug
		void printData();
};

#endif