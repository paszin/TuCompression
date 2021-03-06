#include <optional>

namespace Huffman
{

template <typename D, size_t SIZE>
struct compressedData {
	std::unordered_map<D, std::bitset<SIZE>> dictionary;
    std::vector<std::bitset<SIZE>> compressed;
    std::vector<std::pair<D, D>> bounds;
};

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
std::tuple<	std::unordered_map<D, std::bitset<B>>,
    std::vector<std::bitset<B>>,
    std::vector<std::pair<D, D>>>
compress(const std::vector<D> &column) {
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
			//std::cout << "Bounds" << bounds.first << "   " << bounds.second << '\n';
			boundsAttributeVector.push_back(bounds);
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
	if (!pushed) {
		//std::cout << "Bounds" << bounds.first << "   " << bounds.second << '\n';
		boundsAttributeVector.push_back(bounds);
		attributeVector.push_back(currentBitset);
	}
	return std::tuple(dictionary, attributeVector, boundsAttributeVector);
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


// ---------------------- INTERNAL ------------------ //

/**
	Returns indices to all values in a vector matching the predicate.
*/

template <typename D, std::size_t B>
std::unordered_map<std::bitset<B>, D> getReverseDictionary(std::unordered_map<D, std::bitset<B>> dictionary) {
	std::unordered_map<std::bitset<B>, D> reverseDictionary;
	for (auto const& [k, v] : dictionary) {
		reverseDictionary[v] = k;
	}
	return reverseDictionary;
}

template <typename D, std::size_t B>
std::vector<D> decompressBlock(std::bitset<B> block, std::unordered_map<std::bitset<B>, D> reverseDictionary) {
	std::vector<D> decompressed;
	std::bitset<B> mask;
	size_t shift = 0;
	for (size_t i = 0; i < block.size(); ++i) {
		mask.set(B - 1 - i, 1);
		auto search = ((block & mask) << shift);
		if (search.none() && !block.none()) {
			continue;
		}
		if (reverseDictionary.find(search) != reverseDictionary.end()) {
			shift = i + 1;
			mask.reset();
			decompressed.push_back(reverseDictionary[search]);
		}
	}
	return decompressed;
}



// ---------------------- OPS ------------------ //

/**
	Counts all values matching the predicate.
*/
template <typename D, std::size_t SIZE>
size_t count_where_op_equal(std::unordered_map<D, std::bitset<SIZE>> dictionary,
                            std::vector<std::bitset<SIZE>> compressed,
                            std::vector<std::pair<D, D>> bounds,
                            D value) {
	std::unordered_map<std::bitset<SIZE>, D> reverseDictionary = getReverseDictionary(dictionary);
	int count = 0;
	for (size_t i = 0; i < compressed.size(); i++)
	{
		//std::cout << bounds[i].first << " - " << bounds[i].second << '\n';
		if (bounds[i].first > value || bounds[i].second < value ) {
			continue;
		}
		auto block = decompressBlock<D, SIZE>(compressed[i], reverseDictionary);

		for (size_t j = 0; j < block.size(); j++)
		{
			if (block[j] == value) {
				count++;
			}
		}
	}
	return count;
}


template <typename D, std::size_t SIZE>
size_t count_where_op_range(std::unordered_map<D, std::bitset<SIZE>> dictionary,
                            std::vector<std::bitset<SIZE>> compressed,
                            std::vector<std::pair<D, D>> bounds,
                            std::optional<D> from = std::optional<D>(), std::optional<D> to = std::optional<D>()) {
	std::unordered_map<std::bitset<SIZE>, D> reverseDictionary = getReverseDictionary(dictionary);
	int count = 0;
	for (size_t i = 0; i < compressed.size(); i++)
	{
		//std::cout << bounds[i].first << " - " << bounds[i].second << '\n';
		if ((to && bounds[i].first > to) || (from && bounds[i].second <= from )) {
			continue;
		}
		auto block = decompressBlock<D, SIZE>(compressed[i], reverseDictionary);
		for (size_t j = 0; j < block.size(); j++)
		{
			if ((to && block[j] < to) && (!from || block[j] >= from) ||
			        (from && block[j] >= from) && (!to || block[j] < to))
			{
				count++;
			}
		}

	}
	return count;
}


template <typename D>
D min_op(std::vector<std::pair<D, D>> bounds) {
	D min = bounds[0].first;

	for (size_t i = 0; i < bounds.size(); i++)
	{
		if (bounds[i].first < min) {
			min = bounds[i].first;
		}
	}
	return min;
}

template <typename D>
D max_op(std::vector<std::pair<D, D>> bounds) {
	D max = bounds[0].second;

	for (size_t i = 0; i < bounds.size(); i++)
	{
		if (bounds[i].second > max) {
			max = bounds[i].second;
		}
	}
	return max;
}

template <typename D, std::size_t SIZE>
D sum_where_op_range(std::unordered_map<D, std::bitset<SIZE>> dictionary,
                     std::vector<std::bitset<SIZE>> compressed,
                     std::vector<std::pair<D, D>> bounds,
                     D from = NULL, D to = NULL) {
	std::unordered_map<std::bitset<SIZE>, D> reverseDictionary = getReverseDictionary(dictionary);
	D sum = 0;
	for (size_t i = 0; i < compressed.size(); i++)
	{
		//std::cout << bounds[i].first << " - " << bounds[i].second << '\n';
		if ((to != NULL && bounds[i].first > to) || (from != NULL && bounds[i].second <= from )) {
			continue;
		}
		auto block = decompressBlock<D, SIZE>(compressed[i], reverseDictionary);
		for (size_t j = 0; j < block.size(); j++)
		{
			if ((to != NULL && block[j] < to) && (from == NULL || block[j] >= from) ||
			        (from != NULL && block[j] >= from) && (to == NULL || block[j] < to) ||
			        (from == NULL && to == NULL))
			{
				sum += block[j];
			}
		}

	}
	return sum;
}

template <typename D, std::size_t SIZE>
float avg_op(std::unordered_map<D, std::bitset<SIZE>> dictionary,
             std::vector<std::bitset<SIZE>> compressed) {
	std::unordered_map<std::bitset<SIZE>, D> reverseDictionary = getReverseDictionary(dictionary);
	D sum = 0;
	size_t count = 0;
	for (size_t i = 0; i < compressed.size(); i++)
	{
		auto block = decompressBlock<D, SIZE>(compressed[i], reverseDictionary);
		for (size_t j = 0; j < block.size(); j++)
		{
			sum += block[j];
			count++;
		}

	}
	return (float)sum / (float)count;
}


template <typename D, std::size_t SIZE>
std::vector<D> values_where_range_op(std::unordered_map<D, std::bitset<SIZE>> dictionary,
                                     std::vector<std::bitset<SIZE>> compressed,
                                     std::vector<std::pair<D, D>> bounds,
                                     std::optional<D> from = std::optional<D>(), std::optional<D> to = std::optional<D>()) {
	std::unordered_map<std::bitset<SIZE>, D> reverseDictionary = getReverseDictionary(dictionary);
	std::vector<D> result;
	for (size_t i = 0; i < compressed.size(); i++)
	{
		//std::cout << bounds[i].first << " - " << bounds[i].second << '\n';
		if ((to && bounds[i].first > to) || (from && bounds[i].second <= from )) {
			continue;
		}
		auto block = decompressBlock<D, SIZE>(compressed[i], reverseDictionary);
		for (size_t j = 0; j < block.size(); j++)
		{
			if ((to && block[j] < to) && (!from || block[j] >= from) ||
			        (from && block[j] >= from) && (!to || block[j] < to))
			{
				result.push_back(block[j]);
			}
		}

	}
	return result;
}

template <typename D, std::size_t SIZE>
std::vector<size_t> indexes_where_range_op(std::unordered_map<D, std::bitset<SIZE>> dictionary,
                                      std::vector<std::bitset<SIZE>> compressed,
                                      std::optional<D> from = std::optional<D>(), std::optional<D> to = std::optional<D>()) {
	std::unordered_map<std::bitset<SIZE>, D> reverseDictionary = getReverseDictionary(dictionary);
	std::vector<D> result;
	size_t index = 0;
	for (size_t i = 0; i < compressed.size(); i++)
	{
		auto block = decompressBlock<D, SIZE>(compressed[i], reverseDictionary);
		for (size_t j = 0; j < block.size(); j++)
		{
			if ((to && block[j] < to) && (!from  || block[j] >= from) ||
			        (from && block[j] >= from) && (!to || block[j] < to))
			{
				result.push_back(index);
			}
			index++;
		}

	}
	return result;
}


// ---------------------- BENCHMARK ------------------ //

template <typename D>
Benchmark::CompressionResult benchmark(const std::vector<D> &column, int runs, int warmup, bool clearCache) {
	std::cout << "Huffman - Compressing column" << std::endl;
	auto compressedColumn = compress<D, 64>(column);
	auto compressedPair = std::make_pair(std::get<0>(compressedColumn), std::get<1>(compressedColumn));
	std::cout << "Huffman - Decompressing column" << std::endl;
	assert(column == decompress(compressedPair));
	// std::tuple<std::unordered_map<D, std::bitset<64>>, std::vector<std::bitset<64>>, std::vector<std::pair<D, D>>>
	std::function<std::tuple<std::unordered_map<D, std::bitset<64>>, std::vector<std::bitset<64>>, std::vector<std::pair<D, D>>> ()> compressFunction = [&column]() {
		return compress<D, 64>(column);
	};
	std::function<std::vector<D> ()> decompressFunction = [&compressedPair]() {
		return decompress(compressedPair);
	};
	std::cout << "Huffman - Benchmarking Compression" << std::endl;
	auto compressRuntimes = Benchmark::benchmark(compressFunction, runs, warmup, clearCache);
	std::cout << "Huffman - Benchmarking Decompression" << std::endl;
	auto decompressRuntimes = Benchmark::benchmark(decompressFunction, runs, warmup, clearCache);

	// Compressed Size
	size_t cSize = 0;
	{
		auto dictionary = std::get<0>(compressedColumn);
		auto attributeVector = std::get<1>(compressedColumn);
		auto boundsVector = std::get<2>(compressedColumn);
		cSize += sizeof(dictionary) + sizeof(attributeVector) + sizeof(boundsVector);

		std::unordered_map<D, std::bitset<64>, std::hash<D>, std::equal_to<D>, MyAllocator<std::pair<const D, std::bitset<64>>>> dictionaryWithAlloc(dictionary.begin(), dictionary.end());
		cSize += dictionaryWithAlloc.get_allocator().allocationInByte();
		std::vector<std::bitset<64>, MyAllocator<std::bitset<64>>> attributeVectorWithAlloc(attributeVector.begin(), attributeVector.end());
		cSize += attributeVectorWithAlloc.get_allocator().allocationInByte();
		std::vector<std::pair<D, D>, MyAllocator<std::pair<D, D>>> boundsVectorWithAlloc(boundsVector.begin(), boundsVector.end());
		cSize += boundsVectorWithAlloc.get_allocator().allocationInByte();
	}
	// Uncompressed Size
	std::vector<D, MyAllocator<D>> uncompressedWithAlloc(column.begin(), column.end());
	size_t uSize = uncompressedWithAlloc.get_allocator().allocationInByte();
	uSize += sizeof(column);
	return Benchmark::CompressionResult(compressRuntimes, decompressRuntimes, cSize, uSize);
}

Benchmark::CompressionResult benchmark(const std::vector<std::string> &column, int runs, int warmup, bool clearCache) {
	std::cout << "Huffman - Compressing column" << std::endl;
	auto compressedColumn = compress<std::string, 64>(column);
	auto compressedPair = std::make_pair(std::get<0>(compressedColumn), std::get<1>(compressedColumn));
	std::cout << "Huffman - Decompressing column" << std::endl;
	assert(column == decompress(compressedPair));
	// std::tuple<std::unordered_map<std::string, std::bitset<64>>, std::vector<std::bitset<64>>, std::vector<std::pair<std::string, std::string>>>
	std::function<std::tuple<std::unordered_map<std::string, std::bitset<64>>, std::vector<std::bitset<64>>, std::vector<std::pair<std::string, std::string>>> ()> compressFunction = [&column]() {
		return compress<std::string, 64>(column);
	};
	std::function<std::vector<std::string> ()> decompressFunction = [&compressedPair]() {
		return decompress(compressedPair);
	};
	std::cout << "Huffman - Benchmarking Compression" << std::endl;
	auto compressRuntimes = Benchmark::benchmark(compressFunction, runs, warmup, clearCache);
	std::cout << "Huffman - Benchmarking Decompression" << std::endl;
	auto decompressRuntimes = Benchmark::benchmark(decompressFunction, runs, warmup, clearCache);

	// Compressed Size
	size_t cSize = 0;
	{
		auto dictionary = std::get<0>(compressedColumn);
		auto attributeVector = std::get<1>(compressedColumn);
		auto boundsVector = std::get<2>(compressedColumn);
		cSize += sizeof(dictionary) + sizeof(attributeVector) + sizeof(boundsVector);

		std::unordered_map<std::string, std::bitset<64>, std::hash<std::string>, std::equal_to<std::string>, MyAllocator<std::pair<const std::string, std::bitset<64>>>> dictionaryWithAlloc(dictionary.begin(), dictionary.end());
		cSize += dictionaryWithAlloc.get_allocator().allocationInByte();
		std::vector<std::bitset<64>, MyAllocator<std::bitset<64>>> attributeVectorWithAlloc(attributeVector.begin(), attributeVector.end());
		cSize += attributeVectorWithAlloc.get_allocator().allocationInByte();
		std::vector<std::pair<std::string, std::string>, MyAllocator<std::pair<std::string, std::string>>> boundsVectorWithAlloc(boundsVector.begin(), boundsVector.end());
		cSize += boundsVectorWithAlloc.get_allocator().allocationInByte();

		// Add std::string sizes
		for (const auto &[k, v] : dictionary) {
			cSize += sizeOfString(k);
		}
		for (auto v : boundsVector) {
			cSize += sizeof(v) + sizeOfString(v.first) + sizeOfString(v.second);
		}
	}
	// Uncompressed Size
	std::vector<std::string, MyAllocator<std::string>> uncompressedWithAlloc(column.begin(), column.end());
	size_t uSize = uncompressedWithAlloc.get_allocator().allocationInByte();
	uSize += sizeof(column);
	for (auto v : uncompressedWithAlloc) {
		uSize += sizeOfString(v);
	}
	return Benchmark::CompressionResult(compressRuntimes, decompressRuntimes, cSize, uSize);
}

/**
	D == Dictionary type
	C == Compressed type
	R == OP return type
*/
template <typename D, typename R>
std::vector<size_t> benchmark_op_with_dtype(compressedData<D, 64> &compressedColumn,
	int runs, int warmup, bool clearCache,
    std::function<R (compressedData<D, 64>)> func) {
	std::function<R ()> fn = std::bind(func, compressedColumn);
	return Benchmark::benchmark(fn, runs, warmup, clearCache);
	std::vector<size_t> res;
	return res;
}

} // end namespace Huffman
