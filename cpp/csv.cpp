#include "csv.h"

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

	while(std::getline(infileStream, row, rowSeparator)) {
		if(skipHeader == true) {
			skipHeader = false;
			continue;
		}
		std::stringstream rowStream(row);
		int column = 0;
		while(std::getline(rowStream, cell, cellSeparator)) {
			table[column].push_back(cell);
			column++;
		}
	}
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