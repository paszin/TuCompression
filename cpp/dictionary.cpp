#include "dictionary.h"

/**
	Compresses a column using dictionary encoding and a std::set to store the dictionary.
*/
Dictionary::DictionaryCompressedColumn Dictionary::compress(std::vector<std::string> column) {
	std::set<std::string> dictionary;
	std::vector<std::size_t> attributeVector;

	for(auto cell : column) {
	  dictionary.emplace(cell);
	}

	for(auto cell : column) {
		auto search_it = dictionary.find(cell);
		auto index = std::distance(dictionary.begin(), search_it);
		attributeVector.push_back(index);
	}

	return DictionaryCompressedColumn(dictionary, attributeVector);
}