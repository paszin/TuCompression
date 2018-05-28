#include <queue>
#include <map>



template <typename D>
void buildCodes(const IHuffmanNode* node, std::vector<bool> &prefix, std::map<D, std::vector<bool>> &dictionary) {
    if(const LeafHuffmanNode<D>* lf = dynamic_cast<const LeafHuffmanNode<D>*>(node))
    {
        dictionary[lf->data] = prefix;
    }
    else if (const InternalHuffmanNode<D>* in = dynamic_cast<const InternalHuffmanNode<D>*>(node))
    {
    	std::vector<bool> left = prefix;
    	left.push_back(false);
    	buildCodes<D>(in->left, left, dictionary);
    	std::vector<bool> right = prefix;
    	right.push_back(true);
    	buildCodes<D>(in->right, right, dictionary);
    }
}


template <typename D, typename A>
CompressedColumn<D, A> HuffmanEncoder<D, A>::compress(const std::vector<D> &column) {
	std::set<D> dictionary;
	std::vector<A> attributeVector(column.size());

	auto compare = [](const IHuffmanNode *left, const IHuffmanNode *right){
		return left->frequency > right->frequency;
	};

	// Calculate unique frequencies
	std::map<D, size_t> frequencies;
	for (auto cell : column)
	{
		++frequencies[cell];
	}

	// Insert all leafs in Huffman Tree
	std::priority_queue<IHuffmanNode*, std::vector<IHuffmanNode*>, decltype(compare)> minHeap(compare);
	for (auto const& [key, value] : frequencies)
	{
		minHeap.emplace(new LeafHuffmanNode<D>(value, key));
	}

	// Build Huffman Tree
	while (minHeap.size() != 1)
	{
		auto *left = minHeap.top();
		minHeap.pop();
		auto *right = minHeap.top();
		minHeap.pop();

		minHeap.emplace(new InternalHuffmanNode<D>(left, right));
	}
	std::vector<bool> prefix;
	std::map<D, std::vector<bool>> dictionary2;
	buildCodes<D>(minHeap.top(), prefix, dictionary2);
	for (int i = 0; i < column.size(); ++i)
	{
		attributeVector[i] = dictionary2[column[i]];
	}
	return CompressedColumn(dictionary, attributeVector);
}

template <typename D, typename A>
std::vector<D> HuffmanEncoder<D, A>::decompress(const CompressedColumn<D, A> &column) {
	std::vector<D> decompressed(column.attributeVector.size());
	
	// TODO

	return decompressed;
}

template <typename D, typename A>
size_t HuffmanEncoder<D, A>::size(const CompressedColumn<D, A> &column) {
	// TODO
	return 0;
}