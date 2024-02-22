// common.h

#include <vector>
#include <string>

namespace Common {
	std::string strip(const std::string& s);
	bool empty_line(const std::string& s);
	std::vector<std::string> split(const std::string& s, char delim);
	std::vector<std::vector<std::string>> read_csv(const std::string& path);
}
