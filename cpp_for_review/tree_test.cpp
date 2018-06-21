// This file is a "Hello, world!" in C++ language by Clang for wandbox.
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <string>
#include <chrono>
#include <iostream>
#include <functional>
#include <utility>
#include <queue>
#include <cmath>
#include <cassert>
#include <bitset>
#include <algorithm>

class IHuffmanNode
{
public:
	const size_t frequency;
	virtual ~IHuffmanNode() {}

protected:
	IHuffmanNode(size_t frequency) : frequency(frequency) { }
};

template <typename D>
class InternalHuffmanNode : public IHuffmanNode
{
public:
	const IHuffmanNode *left;
	const IHuffmanNode *right;

	InternalHuffmanNode(IHuffmanNode *left, IHuffmanNode *right) : IHuffmanNode(left->frequency + right->frequency), left(left), right(right) { }
};

template <typename D>
class LeafHuffmanNode : public IHuffmanNode
{
public:
	const D data;

	LeafHuffmanNode(size_t frequency, D data) : IHuffmanNode(frequency), data(data) { }
};

template <typename D>
size_t tree_depth(const IHuffmanNode* node, size_t depth = 0) {
	if (const InternalHuffmanNode<D>* in = dynamic_cast<const InternalHuffmanNode<D>*>(node))
	{
		auto depth1 = tree_depth<D>(in->left, ++depth);
		auto depth2 = tree_depth<D>(in->right, ++depth);
		return std::max(depth1, depth2);
	}
	else {
		return depth;
	}
}

int main(int argc, char* argv[])
{
	// Compile with (gcc 7+):
	// gcc huffman_tree_test.cpp -lstdc++ -O2 -std=c++1z -o tree

	if (argc != 2) {
		std::cerr << "Requiring at least 1 parameter indicating the number of unique values to insert into the tree" << std::endl;
	}

	size_t size = std::stoull(argv[1]);
	std::cout << "Number of uniques: " << size << std::endl;
	std::unordered_map<size_t, size_t> frequencies(size);

	for (size_t i = 0; i < size; ++i)
	{
		frequencies[i] = 1;
	}

	auto compare = [](const IHuffmanNode * left, const IHuffmanNode * right) {
		return left->frequency > right->frequency;
	};

	// Insert all leafs in Huffman Tree
	std::priority_queue<IHuffmanNode*, std::vector<IHuffmanNode*>, decltype(compare)> minHeap(compare);
	for (auto const& [key, value] : frequencies)
	{
		minHeap.emplace(new LeafHuffmanNode<size_t>(value, key));
	}

	// Build Huffman Tree
	while (minHeap.size() != 1)
	{
		auto *left = minHeap.top();
		minHeap.pop();
		auto *right = minHeap.top();
		minHeap.pop();

		minHeap.emplace(new InternalHuffmanNode<size_t>(left, right));
	}

	std::cout << "Depth: " << tree_depth<size_t>(minHeap.top()) << std::endl;
}
