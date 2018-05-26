#include "huffman.h"

CompressedColumn Huffman::compress(std::vector<std::string> column) {
	std::set<std::string> dictionary;
	std::vector<size_t> attributeVector(column.size());

	// TODO

	return CompressedColumn(dictionary, attributeVector);
}

std::vector<std::string> Huffman::decompress(CompressedColumn column) {
	std::vector<size_t> attributeVector(column.attributeVector.begin(), column.attributeVector.end());
	std::vector<std::string> decompressed(column.attributeVector.size());
	
	// TODO

	return decompressed;
}