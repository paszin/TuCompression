template <typename D, typename A>
CompressedColumn<D, A> DictionaryEncoder<D, A>::compress(const std::vector<D> &column) {
	std::set<D> dictionary;
	std::vector<A> attributeVector(column.size());

	for(auto cell : column) {
	  dictionary.emplace(cell);
	}

	int i = 0;
	for(auto cell : column) {
		auto search_it = dictionary.find(cell);
		auto index = std::distance(dictionary.begin(), search_it);
		attributeVector[i] = index;
		i++;
	}

	return CompressedColumn(dictionary, attributeVector);
}

template <typename D, typename A>
std::vector<D> DictionaryEncoder<D, A>::decompress(const CompressedColumn<D, A> &column) {
	std::vector<D> decompressed(column.attributeVector.size());
	int i = 0;
	for(auto cell : column.attributeVector) {
		decompressed[i] = *std::next(column.dictionary.begin(), cell);
		std::cout << "Decompressing " << cell << " to " << decompressed[i] << std::endl;
	}
	return decompressed;
}

template <typename D, typename A>
size_t DictionaryEncoder<D, A>::size(const CompressedColumn<D, A> &column) {
	// TODO
	return 0;
}