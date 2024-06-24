#ifndef WS_CONFIGREFERENCE_HPP
# define WS_CONFIGREFERENCE_HPP

# include <string>
# include <fstream>
# include <vector>
# include <iostream> // ! debug
# include "ws_functions.hpp" // ? might be unnecessary

# define	GLOBAL "global" // config section name for top level

class ConfigReference {
	public:
		ConfigReference(const std::string &reference_file);
		~ConfigReference();
	
		bool keyExists(const std::string section, const std::string keyword);
		bool keyExists(const std::string section, const std::string keyword, size_t &index);
		char keyParamType(const size_t index, const size_t param_num);
		char keyParamType(const std::string section, const std::string keyword, const size_t param_num);
		bool keyParamTypeMatch(const std::string section, const std::string keyword, const size_t param_num, const char type);
		void checkLine(const std::vector<std::string> line);

	private:
		bool processLine(const std::string &line);
		bool validType(const char type);
		bool validType(const char cfg_type, const char ref_type);
		char checkType(const std::string &s);
		std::string typeCharToString(const char c);
		
		std::vector<std::vector<std::string> > _references;
		std::string _section;
	public:
		class ConfigReferenceException : public std::exception {
			public:
				ConfigReferenceException(std::string const msg) : _msg(msg) {}
				const char *what() const noexcept override { return _msg.c_str(); }
			private:
			const std::string _msg;
		};
	public: // ! debug
		void print();
};

#endif