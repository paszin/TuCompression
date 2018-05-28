#pragma once

#include <string>
#include <vector>
#include "compressed.h"

// TODO: How to make members const? Some default ctor error operator=

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


template <typename D, typename A>
class HuffmanEncoder: public Encoder<D, A>
{
public:
	virtual CompressedColumn<D, A> compress(const std::vector<D> &column) override;

	virtual std::vector<D> decompress(const CompressedColumn<D, A> &column) override;

	virtual size_t size(const CompressedColumn<D, A> &column) override;
};

#include "huffman.hpp"