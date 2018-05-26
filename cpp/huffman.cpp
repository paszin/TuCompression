#include "huffman.h"

CompressedColumn Huffman::compress(const std::vector<std::string> &column) const {
	std::set<std::string> dictionary;
	std::vector<size_t> attributeVector(column.size());

	// TODO

	return CompressedColumn(dictionary, attributeVector);
}

std::vector<std::string> Huffman::decompress(const CompressedColumn &column) const {
	std::vector<size_t> attributeVector(column.attributeVector.begin(), column.attributeVector.end());
	std::vector<std::string> decompressed(column.attributeVector.size());
	
	// TODO

	return decompressed;
}

size_t Huffman::size(const CompressedColumn &column) const {
	// TODO
	return 0;
}