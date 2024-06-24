#ifndef WS_WS_FUNCTIONS_HPP
# define WS_WS_FUNCTIONS_HPP
# include <vector>
# include <string>
# include <algorithm> // GCC requires for transform
#include <iostream> // debug

inline bool ws_wspace(const char c) { return (c == ' ' || c == '\t'); }

inline bool ws_keyword_char(const char c) { return (std::isalpha(c) || c == '_' || c == '-' ); }

inline const std::string ws_getword(const std::string &s) {
	size_t end = 0;
	if (s.empty() || !ws_keyword_char(s.at(0)))
		return "";
	while (end < s.size() && ws_keyword_char(s.at(end)) && !ws_wspace(s.at(end)))
		end++;
	return s.substr(0, end);
}

inline bool ws_checkword(const std::string &s, const std::vector<std::string> &list, size_t &index) {
	for (index = 0; index < list.size(); index++) {
		if (s.find(list.at(index)) == 0) {
			return true;
	}	}
	return false;
}

// checks if word is found in vector, searches only until first char in vector elem is not uppercase
inline bool ws_checkword_lower(const std::string &s, const std::vector<std::string> &list, size_t &index) {
	//std::cout << "ws_cw_l: " << s << std::endl;
	for (index = 0; index < list.size() && !std::isupper(list.at(index).at(0)); index++) {
		std::cout << "ws_cw_l iterating elem: " << ws_getword(list.at(index)) << " find result: " << s.find(ws_getword(list.at(index))) << std::endl;
		if (s.find(ws_getword(list.at(index))) == 0)
			return true;
	}
	return (std::cout << "ws_cw_l returning false" << std::endl, false);
	return false;
}

inline bool ws_checkword_lower(const std::string &s, const std::vector<std::string> &list, size_t &index, const size_t start) {
	if (!(start + 1 < list.size()))
		return false;
	std::vector<std::string> sub_list(list.begin() + 1 + start, list.end()); // ! removed + 1
	std::cout << "ws_cw_l sublist begin: " << sub_list.at(0) << std::endl;
	if (ws_checkword_lower(s, sub_list, index))
		return (index += start, true);
	return false;
}

inline std::string ws_toupper(const std::string &s) {
	std::string s_up(s);
	std::transform(s_up.begin(), s_up.end(), s_up.begin(), ::toupper);
	return s_up;
}

inline std::string ws_tolower(const std::string &s) {
	std::string s_lo(s);
	std::transform(s_lo.begin(), s_lo.end(), s_lo.begin(), ::tolower);
	return s_lo;
}

//checks if endline reached (# counts as endline)
inline bool ws_endl(const std::string &s, const size_t pos) {
	if (pos >= s.size() || s.at(pos) == '#')
		return true;
	return false;
}

// gets next non-whitespace character starting from pos, modifies pos
// returns true if found, false if reached end of line
inline bool ws_get_next_nonws(const std::string &s, size_t &pos) {
	while (!ws_endl(s, pos) && ws_wspace(s.at(pos)))
		pos++;
	return !ws_endl(s, pos);
}

// return size of string, excluding comments (comments start with #)
inline size_t ws_size(const std::string &s) {
	size_t end = 0;
	while (end < s.size() && s.at(end) != '#')
		end++;
	return end;
}

// get num:th argument from config defaults line
inline char ws_getarg(size_t num, const std::string &s) {
	size_t pos = 0;
	while (pos < s.size() && !ws_wspace(s.at(pos)))
		pos++;
	while (pos < s.size() && ws_wspace(s.at(pos)))
		pos++;
	while (num > 1) {
		if (pos < s.size() && std::isupper(s.at(pos)) && ++pos) {
			if (pos + 1 == s.size() && s.at(pos - 1) == s.at(pos) && s.at(pos) != 'S')
				return s.at(pos);
			else if (pos == s.size())
				return 0;
			else if (pos < s.size() && !ws_wspace(s.at(pos)))
				throw std::runtime_error("Invalid config default syntax: ws_getarg");
		}
		pos++;
		num--;
	}
	return s.at(pos);
}

// gets nth arg from config line, assumes syntax checked
inline std::string ws_getargstr(size_t num, const std::string &s) {
	if (num == 0) return "";
	size_t pos = 0;
	std::string arg;
	bool quotes = false;
	while (!ws_endl(s, pos) && ws_wspace(s.at(pos)))
		pos++;
	while (!ws_endl(s, pos) && !ws_wspace(s.at(pos)))
		pos++;
	while (num-- > 0) {
		while (!ws_endl(s, pos) && ws_wspace(s.at(pos)))
			pos++;
		while ((!ws_endl(s, pos) || (quotes && pos < s.size())) &&
			(!ws_wspace(s.at(pos)) || quotes) && std::isprint(s.at(pos))) {
				if (s.at(pos) == '"')
					quotes = (!quotes) ? true : false;
				else
					arg.push_back(s.at(pos));
				pos++;
			}
		if (num == 0) return arg;
		else arg = "";
	}
	std::cout << std::endl << "ws_getargstr end, pos: " << pos << std::endl;
	return "";
}

// returns how many arguments a keyword takes, does not count repeats
inline size_t ws_getarglen(const std::string &s) {
	size_t pos = 0;
	size_t len = 0;
	while (pos < s.size() && !ws_wspace(s.at(pos)))
		pos++;
	while (pos < s.size() && ws_wspace(s.at(pos)))
		pos++;
	if (pos == s.size()) return 0;
	while (++len && pos < s.size()) {
		if (!std::isupper(s.at(pos)))
			throw std::runtime_error("Invalid config default syntax:1 ws_getarglen");
		pos++;
		if (ws_endl(s, pos) || (s.at(pos - 1) == s.at(pos) && s.at(pos) != 'S' && ws_endl(s, pos + 1)))
			return len;
		else if (!ws_wspace(s.at(pos)))
			throw std::runtime_error("Invalid config default syntax:2 ws_getarglen");
		pos++;
	}
	throw std::runtime_error("Error at ws_getarglen");
}

inline bool ws_checkend(const std::string &s) {
	size_t pos = 0;
	while (!ws_endl(s, pos) && ws_wspace(s.at(pos)))
		pos++;
	if (!ws_endl(s, pos) && s.at(pos) == '}')
		pos++;
	else
		return false;
	while (!ws_endl(s, pos) && ws_wspace(s.at(pos)))
		pos++;
	return ws_endl(s, pos);
}

// checks string for c, returns position of last instance. max (std::string::npos) if not found
inline size_t ws_getlastchar(const char c, const std::string &s) {
	size_t pos = std::string::npos;
	for (size_t i = 0; i < s.size(); i++)
		if (s.at(i) == c)
			pos = i;
	return pos;
}

#endif