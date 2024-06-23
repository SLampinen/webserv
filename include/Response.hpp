#ifndef WS_RESPONSE_HPP
# define WS_RESPONSE_HPP

# include <string>

// request class that tells webserv how to resolve the request
// returned from Server class

// this class can handle file reading and CGI execution as well, if so desired
// but im coding parser first - rleskine 27.5

# define RES_FILE -1
# define RES_CGI -2
# define RES_DIR -3
# define RES_INVALID -100

class Response {
	public:
		Response();
		Response(const std::string filepath);
		Response(const int code, const std::string filepath);
		Response(const int code, const std::string filepath, const std::string cgipath);
		~Response();

		const int& getType();
		const std::string &getPath();
		const std::string &getCGIPath();

		Response &operator=(const Response assign);

	private:
		int _responsecode;
			// -2 if CGI, -1 if file, positive int if http
		std::string _path;
		std::string _cgi_path;
};

#endif