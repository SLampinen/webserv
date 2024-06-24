#include <iostream> // for debug
#include "ws_functions.hpp"
#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string &filepath) : _ref("cfg/config_defaults") ,_cfg(filepath) {
	_config_sections.push_back(ConfigSection(GLOBAL));
	std::cout << "ConfigParser created" << std::endl;
}

ConfigParser::~ConfigParser() {}

bool ConfigParser::startParse() { std::cout << "CP.STARTPARSE" << std::endl;
	do {
		_cfg.processLine();
		_ref.checkLine(_cfg.getVector());
		if (_cfg.getSection() == GLOBAL && _cfg.getWord(0) == "server" && _cfg.getLastWord() == "{")
			break;
		storeConfigLine();
	} while (_cfg.nextLine());
	if (_cfg.getSection() != GLOBAL || _cfg.getWord(0) != "server" || _cfg.getLastWord() != "{") 
		throw std::runtime_error("Configuration file is missing a server section!");
	return (true);
}

ConfigServer ConfigParser::getServer() { std::cout << "CP.GETSERVER" << std::endl;
	if (_cfg.getSection() != "global" || _cfg.getWord(0) != "server" || _cfg.getLastWord() != "{")
		throw std::runtime_error("Internal error: ConfigParser::getServer executed on non-server begin line");
	_ref.checkLine(_cfg.getVector());
	ConfigServer srv;
	while (_cfg.nextLine() && _cfg.processLine() && _cfg.getSection() != GLOBAL) {
		_ref.checkLine(_cfg.getVector());
		if (_cfg.getWord(0) == "location" && _cfg.getLastWord() == "{") {
			Location add_location(_cfg.getWord(1));
			while (_cfg.nextLine() && _cfg.processLine() && _cfg.getSection() == "location") {
				_ref.checkLine(_cfg.getVector());
				add_location.addConfigLine(_cfg.getVector());
			}
			add_location.initialize();
			srv.addLocation(add_location);
		} else {
			srv.addConfigLine(_cfg.getVector());
		}
	}
	return (srv.initialize(), srv);
}

bool ConfigParser::endParse() {
	if (_cfg.getSection() == GLOBAL && _cfg.getWord(0) == "server" && _cfg.getLastWord() == "{")
		return false;
	bool end_reached = false;
	while ((end_reached = _cfg.nextLine()) && _cfg.processLine()) {
		_ref.checkLine(_cfg.getVector());
		if (_cfg.getSection() == GLOBAL && _cfg.getWord(0) == "server" && _cfg.getLastWord() == "{")
			return false;
		storeConfigLine();
	}
	return !end_reached;
}

bool ConfigParser::storeConfigLine() {
	if (_cfg.getWord(0) == "}")
		return true;
	for (ConfigSection &section : _config_sections) {
		if (section._section_name == _cfg.getSection()) {
			section.addConfigLine(_cfg.getVector());
			return true;
	}	}
	ConfigSection section(_cfg.getSection());
	section.addConfigLine(_cfg.getVector());
	_config_sections.push_back(section);
	return true;
}

void ConfigParser::printCS() {
	for (size_t i = 0; i < _config_sections.size(); i++)
		_config_sections.at(i).printAll();
	std::cout << "END" << std::endl;
}