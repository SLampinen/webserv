#include "ConfigSection.hpp"
#include <iostream> // ! for debug

void ConfigSection::addConfigLine(const std::vector<std::string> line) {
	_config_lines.push_back(line);
}

bool ConfigSection::doesLineExist(const std::string line) {
	for (const std::vector<std::string>& match : _config_lines) {
		if (match.at(1).find(line) == 0)
			return true;
	}
	return false;
}

bool ConfigSection::doesLineExist(const std::string line, size_t &index) {
	for (size_t i = 0; i < _config_lines.size(); i++) {
		if (_config_lines.at(i).at(1).find(line) == 0)
			return (index = i, true);
	}
	return false;
}

bool ConfigSection::doesLineExist(const std::string line, size_t &index, const size_t start) {
	for (size_t i = start + 1; i < _config_lines.size(); i++) {
		if (_config_lines.at(i).at(1).find(line) == 0)
			return (index = i, true);
	}
	return false;
}

std::string const &ConfigSection::getIndexArg(size_t index, size_t num) {
	if (index < _config_lines.size() && ++num < _config_lines.at(index).size())
		return _config_lines.at(index).at(num);
	return _empty;
}

std::string const &ConfigSection::getIndexArg(std::string keyword, size_t num) {
	size_t index;
	if (doesLineExist(keyword, index))
		return (getIndexArg(index, num));
	return _empty;
}

void ConfigSection::printAll() {
	for (size_t i = 0; i < _config_lines.size(); i++) {
		std::cout << "[" << _section_name << "]";
		for (size_t j = 1; j < _config_lines.at(i).size(); j++)
			std::cout << _config_lines.at(i).at(j) << ":";
		std::cout << std::endl;
	}
}