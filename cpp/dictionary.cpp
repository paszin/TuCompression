namespace Dictionary
{
/**
	Compresses a column:
		- Uses a std::vector to create a sorted list of uniques
		- Converts the std::vector to a std::unordered_map to perform O(1) lookups
		- Returns a sorted std::vector as dictionary for decompression
*/
template <typename D, typename C>
std::pair<std::vector<D>, std::vector<C>> compress(const std::vector<D> &column) {
	std::vector<D> dictionary(column.begin(), column.end());
	std::sort(dictionary.begin(), dictionary.end());
	auto last = std::unique(dictionary.begin(), dictionary.end());
	dictionary.erase(last, dictionary.end());

	std::vector<C> attributeVector(column.size());

	std::unordered_map<D, C> lookup(dictionary.size());
	C j = 0;
	for (const auto &key : dictionary) {
		lookup[key] = j;
		++j;
	}

	int i = 0;
	for (const auto &cell : column) {
		auto index = lookup[cell];
		attributeVector[i] = index;
		++i;
	}
	return std::pair(dictionary, attributeVector);
}


/**
	Decompresses a column.
*/
template <typename D, typename C>
std::vector<D> decompress(std::pair<std::vector<D>, std::vector<C>> &compressed) {
	std::vector<D> decompressed;
	decompressed.reserve(compressed.second.size());
	for (auto cell : compressed.second) {
		decompressed.push_back(compressed.first[cell]);
	}
	return decompressed;
}

/**
	Partially decompresses a column. Only the rows in `indices`.
*/
template <typename D, typename C>
std::vector<D> partial_decompress(std::pair<std::vector<D>, std::vector<C>> &compressed, std::vector<size_t> indices) {
	std::vector<D> decompressed;
	decompressed.reserve(indices.size());
	for (auto index : indices) {
		decompressed.push_back(compressed.first[compressed.second[index]]);
	}
	return decompressed;
}

// ---------------------- INTERNAL ------------------ //

/**
	Returns indices to all values in a vector matching the predicate.
*/
template <typename D, typename C>
std::vector<C> vector_view(std::vector<D> &vec, std::function<bool (D)> predicate) {
	std::vector<C> view;
	for (int i = 0; i < vec.size(); ++i)
	{
		if (predicate(vec[i])) {
			view.push_back(i);
		}
	}
	return view;
}

/**
	Search for values matching a predicate.
	1. Create a list of "C" values in the dictionary matching the predicate. (dictionary_view)
	2. Create a copy (copy_view) of all values in the attribute vector whose value is in dictionary_view.
	3. Return copy_view.
*/
template <typename D, typename C>
std::vector<C> where_copy(std::pair<std::vector<D>, std::vector<C>> &compressed, std::function<bool (D)> predicate) {
	// Get indices into dictionary vector that match predicate (index = C)
	auto dictionary_view = vector_view<D, C>(compressed.first, predicate);
	// Filter the attribute vector to only include values in the dictionary
	std::vector<C> copy_view(compressed.second.size());
	std::function<bool (C)> copy_predicate = [&dictionary_view](C i) {
		return std::find(dictionary_view.begin(), dictionary_view.end(), i) != dictionary_view.end();
	};
	// Create a copy of all values in the attribute vector matching the predicate
	auto it = std::copy_if(compressed.second.begin(), compressed.second.end(), copy_view.begin(), copy_predicate);
	copy_view.resize(std::distance(copy_view.begin(), it));
	return copy_view;
}

/**
	Searche for values matching a predicate.
	1. Create a list of "C" values in the dictionary matching the predicate. (dictionary_view)
	2. Create a list of indices (vector_view) of all values in the attribute vector whose value is in dictionary_view.
	3. Return vector_view
*/
template <typename D, typename C>
std::vector<size_t> where_view(std::pair<std::vector<D>, std::vector<C>> &compressed, std::function<bool (D)> predicate) {
	// Get indices into dictionary vector that match predicate (index = A)
	auto dictionary_view = vector_view<D, C>(compressed.first, predicate);
	// Filter the attribute vector to only include values in the dictionary
	std::vector<size_t> vector_view;
	vector_view.reserve(compressed.second.size());
	std::function<bool (C)> view_predicate = [&dictionary_view](C i) {
		return std::find(dictionary_view.begin(), dictionary_view.end(), i) != dictionary_view.end();
	};
	// Create a true view into the attribute vector by getting indices to values matching predicate
	for (size_t i = 0; i < compressed.second.size(); ++i)
	{
		if (view_predicate(compressed.second[i])) {
			vector_view.push_back(i);
		}
	}
	vector_view.resize(vector_view.size());
	return vector_view;
}


// ---------------------- OPS ------------------ //

/**
	Counts all values matching the predicate.
	1. Call where_view().
	2. Calculate size of where_view().
*/
template <typename D, typename C>
size_t count_where_op(std::pair<std::vector<D>, std::vector<C>> &compressed, std::function<bool (D)> predicate) {
	auto attributeVectorWhere = where_view(compressed, predicate);
	return attributeVectorWhere.size();
}

template <typename D, typename C>
D max_op(std::pair<std::vector<D>, std::vector<C>> &compressed) {
	return compressed.first[compressed.first.size() - 1];
}

template <typename D, typename C>
D min_op(std::pair<std::vector<D>, std::vector<C>> &compressed) {
	return compressed.first[0];
}

/**
	Search for values matching a predicate.
	1. Call where_view().
	2. Call partial_decompress().
*/
template <typename D, typename C>
std::vector<D> where_view_op(std::pair<std::vector<D>, std::vector<C>> &compressed, std::function<bool (D)> predicate) {
	auto attributeVectorWhere = where_view(compressed, predicate);
	return partial_decompress(compressed, attributeVectorWhere);
}

/**
	Search for values matching a predicate.
	1. Call where_copy().
	2. Call decompress().
*/
template <typename D, typename C>
std::vector<D> where_copy_op(std::pair<std::vector<D>, std::vector<C>> &compressed, std::function<bool (D)> predicate) {
	auto attributeVectorWhere = where_copy(compressed, predicate);
	auto temp_compressed = std::pair(compressed.first, attributeVectorWhere);
	return decompress(temp_compressed);
}

/**
	Calculates the sum of all values in a column.
*/
template <typename D, typename C>
size_t sum_op(std::pair<std::vector<D>, std::vector<C>> &compressed) {
	size_t total_sum = 0;
	auto attributeVector = compressed.second;
	if (compressed.first.size() == 1) {
		// If we have only a single unique just multiply it with the attribute vector size
		total_sum = compressed.first[0] * compressed.second.size();
	}
	else {
		// Sort the vector and count occurrences. Just decompress old value when new value appears
		std::sort(attributeVector.begin(), attributeVector.end());
		C lastValue = attributeVector[0];
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
	Calculates the sum of all values in a column matching predicate.
	1. Call where_copy().
	2. Call sum_op().
*/
template <typename D, typename C>
size_t sum_where_copy_op(std::pair<std::vector<D>, std::vector<C>> &compressed, std::function<bool (D)> predicate) {
	auto attributeVectorWhere = where_copy(compressed, predicate);
	auto temp_compressed = std::pair(compressed.first, attributeVectorWhere);
	return sum_op(temp_compressed);
}

/**
	Calculates the average of all values in a column.
	IF (size==1) -> average = first element
	ELSE:
		1. Call sum_op().
		2. Divide by total number of elements.
*/
template <typename D, typename C>
float avg_op(std::pair<std::vector<D>, std::vector<C>> &compressed) {
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


// ---------------------- BENCHMARK ------------------ //

/**
	Calls the Benchmark::benchmark functions for compress and decompress with a specific data type for
	the encoding.
*/
template <typename D, typename C>
Benchmark::CompressionResult benchmark_with_dtype(const std::vector<D> &column, int runs, int warmup, bool clearCache) {
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

	// Compressed Size
	// 	Attribute Vector
	std::vector<C, MyAllocator<C>> compressedWithAlloc(compressedColumn.second.begin(), compressedColumn.second.end());
	size_t cSize = compressedWithAlloc.get_allocator().allocationInByte();
	cSize += sizeof(compressedColumn.second);
	//	Dictionary
	std::vector<D, MyAllocator<D>> dictionaryWithAlloc(compressedColumn.first.begin(), compressedColumn.first.end());
	cSize += dictionaryWithAlloc.get_allocator().allocationInByte();
	cSize += sizeof(compressedColumn.first);

	// Uncompressed Size
	std::vector<D, MyAllocator<D>> uncompressedWithAlloc(column.begin(), column.end());
	size_t uSize = uncompressedWithAlloc.get_allocator().allocationInByte();
	uSize += sizeof(column);
	return Benchmark::CompressionResult(compressRuntimes, decompressRuntimes, cSize, uSize);
}


/**
	Calls the Benchmark::benchmark functions for compress and decompress with a specific data type for
	the encoding.
*/
template <typename C>
Benchmark::CompressionResult benchmark_with_dtype(const std::vector<std::string> &column, int runs, int warmup, bool clearCache) {
	auto compressedColumn = compress<std::string, C>(column);
	assert(column == decompress(compressedColumn));
	std::function<std::pair<std::vector<std::string>, std::vector<C>> ()> compressFunction = [&column]() {
		return compress<std::string, C>(column);
	};
	std::function<std::vector<std::string> ()> decompressFunction = [&compressedColumn]() {
		return decompress(compressedColumn);
	};
	std::cout << "Dictionary - Compress Benchmark" << std::endl;
	auto compressRuntimes = Benchmark::benchmark(compressFunction, runs, warmup, clearCache);
	std::cout << "Dictionary - Decompress Benchmark" << std::endl;
	auto decompressRuntimes = Benchmark::benchmark(decompressFunction, runs, warmup, clearCache);

	// Compressed Size
	// 	Attribute Vector
	std::vector<C, MyAllocator<C>> avCompressedWithAlloc(MyAllocator<C>{});
	avCompressedWithAlloc.reserve(compressedColumn.second.size());
	std::copy(compressedColumn.second.begin(), compressedColumn.second.end(), std::back_inserter(avCompressedWithAlloc));
	size_t cSize = avCompressedWithAlloc.get_allocator().allocationInByte();
	//	Dictionary
	std::vector<std::string, MyAllocator<std::string>> dictionaryWithAlloc(MyAllocator<std::string>{});
	dictionaryWithAlloc.reserve(compressedColumn.first.size());
	std::copy(compressedColumn.first.begin(), compressedColumn.first.end(), std::back_inserter(dictionaryWithAlloc));
	cSize += dictionaryWithAlloc.get_allocator().allocationInByte();
	//		Size of individual std::string
	for(auto v : dictionaryWithAlloc) {
		cSize += sizeOfString(v);
	}

	// Uncompressed Size
	std::vector<std::string, MyAllocator<std::string>> uncompressedWithAlloc(MyAllocator<std::string>{});
	uncompressedWithAlloc.reserve(column.size());
	std::copy(column.begin(), column.end(), std::back_inserter(uncompressedWithAlloc));
	size_t uSize = uncompressedWithAlloc.get_allocator().allocationInByte();
	//		Size of individual std::string
	for(auto v : uncompressedWithAlloc) {
		uSize += sizeOfString(v);
	}
	return Benchmark::CompressionResult(compressRuntimes, decompressRuntimes, cSize, uSize);
}


/**
	D == Dictionary type
	C == Compressed type
	R == OP return type
*/
template <typename D, typename C, typename R>
std::vector<size_t> benchmark_op_with_dtype(const std::pair<std::vector<D>, std::vector<C>> &compressedColumn, int runs, int warmup, bool clearCache,
        std::function<R (std::pair<std::vector<D>, std::vector<C>>&)> func) {
	std::function<R ()> fn = std::bind(func, compressedColumn);
	return Benchmark::benchmark(fn, runs, warmup, clearCache);
}
} // end namespace Dictionary