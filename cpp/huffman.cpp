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
		dictionary[lf->data] = prefix;
	}
	else if (const InternalHuffmanNode<D>* in = dynamic_cast<const InternalHuffmanNode<D>*>(node))
	{
		std::bitset<B> left = prefix;
		buildCodes<D>(in->left, left, dictionary, ++depth);
		std::bitset<B> right = prefix;
		right.flip(right.size() - 1 - depth);
		buildCodes<D>(in->right, right, dictionary, ++depth);
	}
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
	std::unordered_map<D, std::bitset<B>> dictionary;
	std::vector<std::bitset<B>> attributeVector(column.size());

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
	auto heap_depth = tree_depth<D>(minHeap.top());
	std::cout << "Depth: " << heap_depth << std::endl;
	std::bitset<B> prefix;
	buildCodes<D>(minHeap.top(), prefix, dictionary);
	for (int i = 0; i < column.size(); ++i)
	{
		attributeVector[i] = dictionary[column[i]];
	}
	return std::pair(dictionary, attributeVector);
}

template <typename D, std::size_t B>
std::vector<D> decompress(std::pair<std::unordered_map<D, std::bitset<B>>, std::vector<std::bitset<B>>> &compressed) {
	std::vector<D> decompressed(compressed.second.size());
	std::unordered_map<std::bitset<B>, D> reverseDictionary;
	for (auto const& [k, v] : compressed.first) {
		reverseDictionary[v] = k;
	}
	for (int i = 0; i < decompressed.size(); ++i)
	{
		decompressed[i] = reverseDictionary[compressed.second[i]];
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
Benchmark::Result benchmark(const std::vector<D> &column, int runs, int warmup, bool clearCache) {
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
	return Benchmark::Result(compressRuntimes, decompressRuntimes, cSize, uSize);
}
} // end namespace Huffman