#include "dictionary.h"

/**
	Compresses a column using dictionary encoding and a std::set to store the dictionary.
*/
CompressedColumn Dictionary::compress(const std::vector<std::string> &column) {
	std::set<std::string> dictionary;
	std::vector<size_t> attributeVector(column.size());

	for(auto cell : column) {
	  dictionary.emplace(cell);
	}

	int i = 0;
	for(auto cell : column) {
		auto search_it = dictionary.find(cell);
		auto index = std::distance(dictionary.begin(), search_it);
		attributeVector[i] = index;
		i++;
	}

	return CompressedColumn(dictionary, attributeVector);
}

std::vector<std::string> Dictionary::decompress(const CompressedColumn &column) {
	std::vector<size_t> attributeVector(column.attributeVector.begin(), column.attributeVector.end());
	std::vector<std::string> decompressed(column.attributeVector.size());
	int i = 0;
	for(auto cell : column.attributeVector) {
		decompressed[i] = attributeVector[cell];
	}
	return decompressed;
}

size_t Dictionary::size(const CompressedColumn &column) {
	// TODO
	return 0;
}