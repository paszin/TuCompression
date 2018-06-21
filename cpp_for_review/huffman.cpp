namespace Huffman
{
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

template <typename D, std::size_t B>
void buildCodes(const IHuffmanNode* node, std::bitset<B> &prefix, std::unordered_map<D, std::bitset<B>> &dictionary, size_t depth = 0) {
	if (const LeafHuffmanNode<D>* lf = dynamic_cast<const LeafHuffmanNode<D>*>(node))
	{
		prefix.set(prefix.size() - depth - 1, 1); // add a 1 to the end
		dictionary[lf->data] = prefix;
	}
	else if (const InternalHuffmanNode<D>* in = dynamic_cast<const InternalHuffmanNode<D>*>(node))
	{
		std::bitset<B> left = prefix;
		buildCodes<D>(in->left, left, dictionary, depth + 1);
		std::bitset<B> right = prefix;
		right.flip(right.size() - 1 - depth);
		buildCodes<D>(in->right, right, dictionary, depth + 1);
	}
}

template <std::size_t B>
int getCodeLength(std::bitset<B> bs) {
	//returns index of last 1 in the bitset
	for (std::size_t i = 0; i < bs.size(); ++i) {
		if (bs[i] == 1) {
			return B - i;
		}
	}
	return 0;
}

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

template <typename D, std::size_t B>
std::pair<std::unordered_map<D, std::bitset<B>>, std::vector<std::bitset<B>>> compress(const std::vector<D> &column) {
	auto compare = [](const IHuffmanNode * left, const IHuffmanNode * right) {
		return left->frequency > right->frequency;
	};

	// Calculate unique frequencies
	std::unordered_map<D, size_t> frequencies;
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

	// Build dictionary
	std::unordered_map<D, std::bitset<B>> dictionary;
	std::bitset<B> prefix;
	buildCodes<D>(minHeap.top(), prefix, dictionary);

	// Compress Attribute Vector
	std::vector<std::bitset<B>> attributeVector;
	std::vector<std::pair<D, D>> boundsAttributeVector;
	int bitsetLength = 0;
	int codeLength;
	std::bitset<B> currentBitset;
	std::pair<D, D> bounds;
	bool pushed = false;
	bool firstRun = true;
	for (int i = 0; i < column.size(); ++i)
	{
		pushed = false;
		std::bitset<B> code = dictionary[column[i]];
		codeLength = getCodeLength(code);
		if (bitsetLength + getCodeLength(code) > B) {
			bitsetLength = 0;
			attributeVector.push_back(currentBitset);
			currentBitset.reset();
			pushed = true;
			firstRun = true;
		}
		code >>= bitsetLength; //shift
		currentBitset |= code; //bitwise or
		bitsetLength += codeLength;
		// save min and max value to bounds
		if (firstRun) {
				bounds.first = column[i];
				bounds.second = column[i];
				//std::cout << "Bounds" << bounds.first << "   " << bounds.second << '\n';
			} else {
				bounds.first = std::min(bounds.first, column[i]);
				bounds.second = std::max(bounds.second, column[i]);
				//std::cout << "Bounds" << bounds.first << "   " << bounds.second << '\n';
			}
			firstRun = false;
	}
	if(!pushed) {
		boundsAttributeVector.push_back(bounds);
		attributeVector.push_back(currentBitset);
	}
	return std::pair(dictionary, attributeVector);
}


template <typename D, std::size_t B>
std::vector<D> decompress(std::pair<std::unordered_map<D, std::bitset<B>>, std::vector<std::bitset<B>>> &compressed) {
	std::unordered_map<std::bitset<B>, D> reverseDictionary;
	for (auto const& [k, v] : compressed.first) {
		reverseDictionary[v] = k;
	}
	std::vector<D> decompressed;
	for (auto bitset : compressed.second) {
		std::bitset<B> mask;
		size_t shift = 0;
		for (size_t i = 0; i < bitset.size(); ++i) {

			mask.set(63 - i, 1); // 63 = B - 1
			auto search = ((bitset & mask) << shift);
			if (search.none() && !bitset.none()) {
				continue;
			}
			if (reverseDictionary.find(search) != reverseDictionary.end()) {
				shift = i + 1;
				mask.reset();
				decompressed.push_back(reverseDictionary[search]);
			}
		}
	}
	return decompressed;
}


template <typename D, std::size_t B>
size_t compressedSize(std::pair<std::unordered_map<D, std::bitset<B>>, std::vector<std::bitset<B>>> &compressed) {
	size_t size = 0;
	// Size of dictionary + attribute vector data structures => std::unordered_map + std::vector
	size += sizeof(compressed.first) + sizeof(compressed.second);
	// Size of dictionary values
	for (auto const& [k, v] : compressed.first) {
		// D + std::bitset<B>
		size += sizeof(k) + sizeof(v);
	}
	// Size of attribute vector values
	for (auto attribute : compressed.second) {
		// std::bitest<B>
		size += sizeof(attribute);
	}
	return size;
}

template <typename D>
Benchmark::CompressionResult benchmark(const std::vector<D> &column, int runs, int warmup, bool clearCache) {
	std::cout << "Huffman - Compressing column" << std::endl;
	auto compressedColumn = compress<D, 64>(column);
	std::cout << "Huffman - Decompressing column" << std::endl;
	assert(column == decompress(compressedColumn));
	std::function<std::pair<std::unordered_map<D, std::bitset<64>>, std::vector<std::bitset<64>>> ()> compressFunction = [&column]() {
		return compress<D, 64>(column);
	};
	std::function<std::vector<D> ()> decompressFunction = [&compressedColumn]() {
		return decompress(compressedColumn);
	};
	std::cout << "Huffman - Benchmarking Compression" << std::endl;
	auto compressRuntimes = Benchmark::benchmark(compressFunction, runs, warmup, clearCache);
	std::cout << "Huffman - Benchmarking Decompression" << std::endl;
	auto decompressRuntimes = Benchmark::benchmark(decompressFunction, runs, warmup, clearCache);
	size_t cSize = compressedSize(compressedColumn);
	size_t uSize = (sizeof(column) + sizeof(D) * column.size());
	return Benchmark::CompressionResult(compressRuntimes, decompressRuntimes, cSize, uSize);
}
} // end namespace Huffman