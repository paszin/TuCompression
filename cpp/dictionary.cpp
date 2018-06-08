namespace Dictionary
{
/**
	Compres:
		- Uses a std::set to create a sorted list of uniques
		- Converts the std::set to a std::unordered_map to perform O(1) lookups
		- Returns a std::vector as dictionary for decompression
*/
template <typename D, typename A>
std::pair<std::vector<D>, std::vector<A>> compress(const std::vector<D> &column) {
	std::set<D> dictionary;
	std::vector<A> attributeVector(column.size());

	for (auto cell : column) {
		dictionary.emplace(cell);
	}

	std::unordered_map<D, A> lookup;
	A j = 0;
	for (auto key : dictionary) {
		lookup[key] = j;
		++j;
	}

	int i = 0;
	for (auto cell : column) {
		auto index = lookup[cell];
		attributeVector[i] = index;
		++i;
	}
	return std::pair(std::vector<D>(dictionary.begin(), dictionary.end()), attributeVector);
}


template <typename D, typename A>
std::vector<D> decompress(std::pair<std::vector<D>, std::vector<A>> &compressed) {
	std::vector<D> decompressed;
	decompressed.reserve(compressed.second.size());
	for (auto cell : compressed.second) {
		decompressed.push_back(compressed.first[cell]);
	}
	return decompressed;
}

template <typename D, typename A>
std::vector<D> partial_decompress(std::pair<std::vector<D>, std::vector<A>> &compressed, std::vector<size_t> indices) {
	std::vector<D> decompressed;
	decompressed.reserve(indices.size());
	for (auto index : indices) {
		decompressed.push_back(compressed.first[compressed.second[index]]);
	}
	return decompressed;
}


template <typename D, typename A>
size_t compressedSize(std::pair<std::vector<D>, std::vector<A>> &compressed) {
	size_t size = 0;
	// Size of data structures
	size += sizeof(compressed.first) + sizeof(compressed.second);
	// Size of dictionary values
	size += sizeof(D) * compressed.first.size();
	// Size of attribute vector values
	size += sizeof(A) * compressed.second.size();
	return size;
}

template <typename D, typename A>
size_t sum_op(std::pair<std::vector<D>, std::vector<A>> &compressed) {
	size_t total_sum = 0;
	auto attributeVector = compressed.second;
	if (compressed.first.size() == 1) {
		// If we have only a single unique just multiply it with the attribute vector size
		total_sum = compressed.first[0] * compressed.second.size();
	}
	else {
		// Sort the vector and count occurrences. Just decompress old value when new value appears
		std::sort(attributeVector.begin(), attributeVector.end());
		A lastValue = attributeVector[0];
		size_t lastValueCount = 1;
		for (int i = 1; i < attributeVector.size(); ++i)
		{
			if (lastValue == attributeVector[i]) {
				++lastValueCount;
			}
			else if (lastValue != attributeVector[i]) {
				// New value
				total_sum += compressed.first[lastValue] * lastValueCount;
				lastValue = attributeVector[i];
				lastValueCount = 1;
			}
			if (i + 1 == attributeVector.size()) {
				// End of loop
				total_sum += compressed.first[lastValue] * lastValueCount;
			}
		}
	}
	return total_sum;
}

/**
	Returns indices to values in vec matching the predicate.
*/
template <typename D, typename A>
std::vector<A> vector_view(std::vector<D> &vec, std::function<bool (D)> predicate) {
	std::vector<A> view;
	for (int i = 0; i < vec.size(); ++i)
	{
		if (predicate(vec[i])) {
			view.push_back(i);
		}
	}
	return view;
}


template <typename D, typename A>
size_t sum_with_predicate_op(std::pair<std::vector<D>, std::vector<A>> &compressed, std::function<bool (D)> predicate) {
	// Get indices into dictionary vector that match predicate (index = A)
	auto dictionary_view = vector_view<D, A>(compressed.first, predicate);
	// Filter the attribute vector to only include values in the dictionary
	std::vector<A> filteredAttributeVector(compressed.second.size());
	std::function<bool (A)> copy_predicate = [&dictionary_view](A i) {
		return std::find(dictionary_view.begin(), dictionary_view.end(), i) != dictionary_view.end();
	};
	// TODO: Is is possible to just create a view into the vector?
	auto it = std::copy_if(compressed.second.begin(), compressed.second.end(), filteredAttributeVector.begin(), copy_predicate);
	filteredAttributeVector.resize(std::distance(filteredAttributeVector.begin(), it));
	// Calculate sum on filered attribute vector
	// TODO: Is this possible without copying first and second (std::reference_wrapper?)
	auto temp_compressed = std::pair(compressed.first, filteredAttributeVector);
	size_t total_sum = sum_op(temp_compressed);
	return total_sum;
}


template <typename D, typename A>
float avg_op(std::pair<std::vector<D>, std::vector<A>> &compressed) {
	float avg = 0;
	if (compressed.first.size() == 1) {
		// If we have only a single unique then the avg is that value
		avg = compressed.first[0];
	}
	else {
		// Build total sum and divide by number of values
		auto total_sum = sum_op(compressed);
		avg = (float)total_sum / (float)compressed.second.size();
	}
	return avg;
}

/**
	Search for values matching a predicate.
	1. Create a list of "A" values in the dictionary matching the predicate. (dictionary_view)
	2. Create a copy of all values in the attribute vector whose value is in dictionary_view.
	3. Decompress.
	Uses decompress.
*/
template <typename D, typename A>
std::vector<D> search(std::pair<std::vector<D>, std::vector<A>> &compressed, std::function<bool (D)> predicate) {
	// Get indices into dictionary vector that match predicate (index = A)
	auto dictionary_view = vector_view<D, A>(compressed.first, predicate);
	// Filter the attribute vector to only include values in the dictionary
	std::vector<A> filteredAttributeVector(compressed.second.size());
	std::function<bool (A)> copy_predicate = [&dictionary_view](A i) {
		return std::find(dictionary_view.begin(), dictionary_view.end(), i) != dictionary_view.end();
	};
	// TODO: Is is possible to just create a view into the vector?
	auto it = std::copy_if(compressed.second.begin(), compressed.second.end(), filteredAttributeVector.begin(), copy_predicate);
	filteredAttributeVector.resize(std::distance(filteredAttributeVector.begin(), it));
	auto temp_compressed = std::pair(compressed.first, filteredAttributeVector);
	return decompress(temp_compressed);
}


/**
	Searche for values matching a predicate.
	1. Create a list of "A" values in the dictionary matching the predicate. (dictionary_view)
	2. Create a list of indices of all values in the attribute vector whose value is in dictionary_view.
	3. Decompress
	Uses partial_decompress.
*/
template <typename D, typename A>
std::vector<D> search_v2(std::pair<std::vector<D>, std::vector<A>> &compressed, std::function<bool (D)> predicate) {
	// Get indices into dictionary vector that match predicate (index = A)
	auto dictionary_view = vector_view<D, A>(compressed.first, predicate);
	// Filter the attribute vector to only include values in the dictionary
	std::vector<size_t> attributeVector_view;
	attributeVector_view.reserve(compressed.second.size());
	std::function<bool (A)> view_predicate = [&dictionary_view](A i) {
		return std::find(dictionary_view.begin(), dictionary_view.end(), i) != dictionary_view.end();
	};
	// Create a true view into the attribute vector by getting indices to values matching predicate
	for (size_t i = 0; i < compressed.second.size(); ++i)
	{
		if(view_predicate(compressed.second[i])) {
			attributeVector_view.push_back(i);
		}
	}
	attributeVector_view.resize(attributeVector_view.size());
	return partial_decompress(compressed, attributeVector_view);
}


template <typename D, typename C>
Benchmark::Result benchmark_with_dtype(const std::vector<D> &column, int runs, int warmup, bool clearCache) {
	auto compressedColumn = compress<D, C>(column);
	assert(column == decompress(compressedColumn));
	std::function<std::pair<std::vector<D>, std::vector<C>> ()> compressFunction = [&column]() {
		return compress<D, C>(column);
	};
	std::function<std::vector<D> ()> decompressFunction = [&compressedColumn]() {
		return decompress(compressedColumn);
	};
	std::cout << "Dictionary - Compress Benchmark" << std::endl;
	auto compressRuntimes = Benchmark::benchmark(compressFunction, runs, warmup, clearCache);
	std::cout << "Dictionary - Decompress Benchmark" << std::endl;
	auto decompressRuntimes = Benchmark::benchmark(decompressFunction, runs, warmup, clearCache);
	size_t cSize = compressedSize(compressedColumn);
	size_t uSize = (sizeof(column) + sizeof(D) * column.size());
	return Benchmark::Result(compressRuntimes, decompressRuntimes, cSize, uSize);
}


template <typename D>
Benchmark::Result benchmark(const std::vector<D> &column, int runs, int warmup, bool clearCache) {
	std::set<D> uniques;
	for (auto cell : column) {
		uniques.emplace(cell);
	}
	if (uniques.size() <= std::pow(2, 8)) {
		std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^8" << std::endl;
		return benchmark_with_dtype<D, uint8_t>(column, runs, warmup, clearCache);
	}
	else if (uniques.size() <= std::pow(2, 16))
	{
		std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^16" << std::endl;
		return benchmark_with_dtype<D, uint16_t>(column, runs, warmup, clearCache);
	}
	else if (uniques.size() <= std::pow(2, 32))
	{
		std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^32" << std::endl;
		return benchmark_with_dtype<D, uint32_t>(column, runs, warmup, clearCache);
	}
	else if (uniques.size() <= std::pow(2, 64))
	{
		std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^64" << std::endl;
		return benchmark_with_dtype<D, uint64_t>(column, runs, warmup, clearCache);
	}
	else {
		std::cout << "Cannot address more than 2^64 uniques" << std::endl;
	}
}
} // end namespace Dictionary