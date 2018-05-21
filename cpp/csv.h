#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>

namespace CSV {
	std::vector<std::string> headerFromFile(std::string filePath);

	std::vector<std::vector<std::string>> toColumnStore(std::string filePath, bool skipHeader=true);

	std::vector<std::vector<std::string>> toRowStore(std::string filePath, bool skipHeader=true);
}