#include <algorithm>


std::vector<std::string> CSV::headerFromFile(std::string filePath) {
	std::vector<std::string> header;
	std::ifstream infileStream(filePath);
	
	std::string row;
	std::string cell;

	char rowSeparator = '\n';
	char cellSeparator = '|';

	while(std::getline(infileStream, row, rowSeparator)) {
		std::stringstream rowStream(row);
		std::vector<std::string> splitRow;
		while(std::getline(rowStream, cell, cellSeparator)) {
			header.push_back(cell);
		}
		break;
	}
	return header;
}

std::vector<std::vector<std::string>> CSV::toColumnStore(std::string filePath, bool skipHeader) {
	std::ifstream infileStream(filePath);

	std::string row;
	std::string cell;

	char rowSeparator = '\n';
	char cellSeparator = '|';

	std::size_t columns = CSV::headerFromFile(filePath).size();
	std::vector<std::vector<std::string>> table(columns);

	size_t rowCount = 0;
	while(std::getline(infileStream, row, rowSeparator)) {
		if(skipHeader == true) {
			skipHeader = false;
			continue;
		}
		std::stringstream rowStream(row);
		size_t column = 0;
		while(std::getline(rowStream, cell, cellSeparator)) {
			table[column].push_back(cell);
			++column;
		}
		++rowCount;
	}
	// std::cout << std::endl;
	return table;
}

std::vector<std::vector<std::string>> CSV::toRowStore(std::string filePath, bool skipHeader) {
	std::vector<std::vector<std::string>> table;
	std::ifstream infileStream(filePath);

	std::string row;
	std::string cell;

	char rowSeparator = '\n';
	char cellSeparator = '|';

	while(std::getline(infileStream, row, rowSeparator)) {
		if(skipHeader == true) {
			skipHeader = false;
			continue;
		}
		std::stringstream rowStream(row);
		std::vector<std::string> splitRow;
		while(std::getline(rowStream, cell, cellSeparator)) {
			splitRow.push_back(cell);
		}
		table.push_back(splitRow);
	}
	return table;
}

template <typename T>
void CSV::writeLine(std::vector<std::string> &header, std::vector<T> &line, std::string filename) {
	std::ofstream file;
	file.open(filename);
	for (int i = 0; i < header.size(); ++i)
	{
		file << header[i];
		if(i+1 < header.size()) {
			file << ",";
		}
	}
	file << "\n";
	for (int i = 0; i < line.size(); ++i)
	{
		file << line[i];
		if(i+1 < line.size()) {
			file << ",";
		}
	}
	file << "\n";
	file.close();
}

template <typename T>
void CSV::writeMultiLine(std::vector<std::string> &header, std::vector<std::vector<T>> &lines, std::string filename) {
	std::ofstream file;
	file.open(filename);
	for (int i = 0; i < header.size(); ++i)
	{
		file << header[i];
		if(i+1 < header.size()) {
			file << ",";
		}
	}
	file << "\n";
	for (int i = 0; i < lines[0].size(); ++i)
	{
		for (int j = 0; j < lines.size(); ++j)
		{
			file << lines[j][i];
			if(j+1 < lines.size()) {
				file << ",";
			}
		}
		file << "\n";
	}
	file.close();
}