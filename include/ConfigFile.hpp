#ifndef WS_CONFIGFILE_HPP
# define WS_CONFIGFILE_HPP

# include <string>
# include <fstream>
# include <vector>
# include <iostream> // ! debug
# include "ConfigReference.hpp" // for GLOBAL definition
# include "ws_functions.hpp" // ? might be unnecessary

class ConfigFile {
	public:
		ConfigFile(const std::string &config_file);
		~ConfigFile();
	
		// ! check pub priv
		bool emptyLine();
		bool nextLine();
		bool processLine();
		
		const std::string &getSection();
		const std::string &getWord(const size_t &num);
		const std::string &getLastWord();
		std::vector<std::string> getVector();

	private:
		bool processParam(size_t &pos);
		bool validType(const char type);
		
		std::ifstream _file;
		std::string _line;
		std::vector<std::string> _words;
		std::vector<std::string> _sections;
		const std::string empty = "";
	public:
		class ConfigFileException : public std::exception {
			public:
				ConfigFileException(std::string const msg) : _msg(msg) {}
				const char *what() const noexcept override { return _msg.c_str(); }
			private:
			const std::string _msg;
		};
	public: // ! debug
		void print();
};

#endif