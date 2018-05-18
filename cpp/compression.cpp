#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>

std::vector<std::string> readCSVHeader(std::string filePath) {
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

std::vector<std::vector<std::string>> readCSVToColumnStore(std::string filePath, bool skipHeader=true) {
	std::ifstream infileStream(filePath);

	std::string row;
	std::string cell;

	char rowSeparator = '\n';
	char cellSeparator = '|';

	std::size_t columns = readCSVHeader(filePath).size();
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

std::vector<std::vector<std::string>> readCSVToRowStore(std::string filePath, bool skipHeader=true) {
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

int main(int argc, char* argv[]) {
	std::string dataFile = "../data/order.tbl";
	auto header = readCSVHeader(dataFile);
	std::cout << "HEADER" << std::endl;
	for(auto cell : header) {
		std::cout << cell << std::endl;
	}
	auto table = readCSVToColumnStore(dataFile);

	for(auto column : table) {
		for(auto cell : column) {
			std::cout << cell << std::endl;
		}
	}

	// for(auto row : table) {
	// 	for(auto cell : row) {
	// 		std::cout << cell << std::endl;
	// 	}
	// }
}
