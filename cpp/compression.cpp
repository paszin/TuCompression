#include <string>
#include "csv.h"
#include "dictionary.h"
#include "huffman.h"



int main(int argc, char* argv[]) {
	std::string dataFile = "../data/order.short.tbl";
	auto table = CSV::toColumnStore(dataFile);

	std::vector<Dictionary::DictionaryCompressedColumn> compressedTable;
	for(auto column : table) {
		auto compressedColumn = Dictionary::compress(column);
		compressedTable.push_back(compressedColumn);
	}

	// gcc version 7.3.0 (MinGW-W64 project)
	// Command to compile code (Windows syntax!):
	// gcc .\compression.cpp .\csv.cpp .\dictionary.cpp .\huffman.cpp -lstdc++ -o compression; .\compression.exe
}
