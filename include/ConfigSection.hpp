#ifndef WS_CONFIGSECTION_HPP
# define WS_CONFIGSECTION_HPP

#include <string>
#include <vector>

// base class for config sections such as Server
class ConfigSection {
	public:
		ConfigSection(const std::string section_name) : _section_name(section_name) {};

		// methods for setting up
		void addConfigLine(const std::vector<std::string> line);

		// accessed after setup
		bool doesLineExist(const std::string line);
		bool doesLineExist(const std::string line, size_t &index);
		bool doesLineExist(const std::string line, size_t &index, const size_t start);
		std::string const &getIndexArg(size_t index, size_t num);
		std::string const &getIndexArg(std::string keyword, size_t num);

		const std::string _section_name;
		const std::string _empty = "";
	private:
		std::vector<std::vector<std::string> > _config_lines;
};

#endif