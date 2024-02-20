#ifndef __COMMON
#define __COMMON

#include <vector>
#include <string>

namespace Common {
	std::string strip(const std::string& s);
	bool empty_line(const std::string& s);
	std::vector<std::string> split(const std::string& s, char delim);
	std::vector<std::vector<std::string>> read_csv(const std::string& path);
	void write_csv(const std::vector<std::string>& v, std::ostream& os);
}
#endif
