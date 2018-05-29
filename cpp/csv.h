#pragma once

#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>

namespace CSV {
	std::vector<std::string> headerFromFile(std::string filePath);

	std::vector<std::vector<std::string>> toColumnStore(std::string filePath, bool skipHeader=true);

	std::vector<std::vector<std::string>> toRowStore(std::string filePath, bool skipHeader=true);

	template <typename T>
	void writeLine(std::vector<std::string> &header, std::vector<T> &line, std::string filename);

	template <typename T>
	void writeMultiLine(std::vector<std::string> &header, std::vector<std::vector<T>> &lines, std::string filename);
}

#include "csv.hpp"