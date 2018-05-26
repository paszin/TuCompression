#pragma once

#include <string>
#include <set>
#include <vector>

struct CompressedColumn
{
	std::set<std::string> dictionary;
	std::vector<size_t> attributeVector;

	CompressedColumn(
		std::set<std::string> dictionary,
		std::vector<size_t> attributeVector) : dictionary(dictionary), attributeVector(attributeVector) { }
};