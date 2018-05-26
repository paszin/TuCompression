#pragma once

#include <set>
#include <vector>

template <typename D, typename A>
struct CompressedColumn
{
	std::set<D> dictionary;
	std::vector<A> attributeVector;

	CompressedColumn(
		std::set<D> dictionary,
		std::vector<A> attributeVector) : dictionary(dictionary), attributeVector(attributeVector) { }
};