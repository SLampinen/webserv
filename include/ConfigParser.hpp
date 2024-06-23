#ifndef WS_CONFIGPARSER_HPP
# define WS_CONFIGPARSER_HPP

# include <string>
# include <fstream>
# include <vector>
# include "ConfigReference.hpp" // ! GLOBAL defined here
# include "ConfigFile.hpp"
# include "ConfigServer.hpp"

// usage: create instance with config file path, 
// start parsing with startParse() which returns true if valid conf
// then run endParse which will return true if parsing has finished, if not then
// get a Server instance with getServer()
// // get servers with getServer(), returns false when last server reached
// // finish parse with endParse(), returns true if parsing finished successfully
class ConfigParser {
	public:
		ConfigParser(const std::string &filepath);
		~ConfigParser();

		bool startParse();
		ConfigServer getServer();
		bool endParse();

	private:
		bool storeConfigLine();
		
		std::vector<ConfigSection> _config_sections;
		ConfigReference _ref;
		ConfigFile _cfg;
	public: // ! debug
		void printCS();
};

#endif