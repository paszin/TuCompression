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
#include "csv.h"


namespace Benchmark
{
	template <typename C>
	std::vector<size_t> benchmark(std::function<C ()> wrapperFunction, int runs, int warmup, bool clearCache) {
		if (clearCache == false)
		{
			for (int i = 0; i < warmup; ++i)
			{
				volatile auto compressed = wrapperFunction();
			}
		}
		auto runTimes = std::vector<size_t>(runs);
		for (int i = 0; i < runs; ++i)
		{
			std::cout << "\rBenchmarking - Run: " << i;
			if (clearCache == true)
			{
				// TODO: call mem_flush. Do it like this?
				// mem_flush(&column, sizeof(column));
			}
			auto start = std::chrono::system_clock::now();
			volatile auto compressed = wrapperFunction();
			auto end = std::chrono::system_clock::now();
			runTimes[i] = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		}
		std::cout << std::endl;
		return runTimes;
	}

	struct Result
	{
		const std::vector<size_t> compressionTimes;
		const std::vector<size_t> decompressionTimes;
		const size_t compressedSize;
		const size_t uncompressedSize;
		const double compressionRatio;

		Result(std::vector<size_t> cT, std::vector<size_t> dT, size_t cSize, size_t uSize) :
			compressionTimes(cT), decompressionTimes(dT), compressedSize(cSize), uncompressedSize(uSize),
			compressionRatio((double)uSize / (double)cSize) {}
	};
}


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

		for(auto cell : column) {
			dictionary.emplace(cell);
		}

		std::unordered_map<D, A> lookup;
		A j = 0;
		for(auto key : dictionary) {
			lookup[key] = j;
			++j;
		}

		int i = 0;
		for(auto cell : column) {
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
		for (int i = 0; i < compressed.second.size(); ++i)
		{
			decompressed[i] = compressed.first[compressed.second[i]];
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


	template <typename D>
	Benchmark::Result benchmark(const std::vector<D> &column, int runs, int warmup, bool clearCache) {
		std::set<D> uniques;
		for(auto cell : column) {
			uniques.emplace(cell);
		}
		if(uniques.size() <= std::pow(2, 8)) {
			std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^8" << std::endl;
			auto compressedColumn = compress<D, uint8_t>(column);
			std::function<std::pair<std::vector<D>, std::vector<uint8_t>> ()> compressFunction = [&column]() {
				return compress<D, uint8_t>(column);
			};
			std::function<std::vector<D> ()> decompressFunction = [&compressedColumn]() {
				return decompress(compressedColumn);
			};
			std::cout << "Dictionary - Benchmarking Compression" << std::endl;
			auto compressRuntimes = Benchmark::benchmark(compressFunction, runs, warmup, clearCache);
			std::cout << "Dictionary - Benchmarking Decompression" << std::endl;
			auto decompressRuntimes = Benchmark::benchmark(decompressFunction, runs, warmup, clearCache);
			size_t cSize = compressedSize(compressedColumn);
			size_t uSize = (sizeof(column) + sizeof(D) * column.size());
			return Benchmark::Result(compressRuntimes, decompressRuntimes, cSize, uSize);
		}
		else if (uniques.size() <= std::pow(2, 16))
		{
			std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^16" << std::endl;
			auto compressedColumn = compress<D, uint16_t>(column);
			std::function<std::pair<std::vector<D>, std::vector<uint16_t>> ()> compressFunction = [&column]() {
				return compress<D, uint16_t>(column);
			};
			std::function<std::vector<D> ()> decompressFunction = [&compressedColumn]() {
				return decompress(compressedColumn);
			};
			auto compressRuntimes = Benchmark::benchmark(compressFunction, runs, warmup, clearCache);
			auto decompressRuntimes = Benchmark::benchmark(decompressFunction, runs, warmup, clearCache);
			size_t cSize = compressedSize(compressedColumn);
			size_t uSize = (sizeof(column) + sizeof(D) * column.size());
			return Benchmark::Result(compressRuntimes, decompressRuntimes, cSize, uSize);
		}
		else if (uniques.size() <= std::pow(2, 32))
		{
			std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^32" << std::endl;
			auto compressedColumn = compress<D, uint32_t>(column);
			std::function<std::pair<std::vector<D>, std::vector<uint32_t>> ()> compressFunction = [&column]() {
				return compress<D, uint32_t>(column);
			};
			std::function<std::vector<D> ()> decompressFunction = [&compressedColumn]() {
				return decompress(compressedColumn);
			};
			auto compressRuntimes = Benchmark::benchmark(compressFunction, runs, warmup, clearCache);
			auto decompressRuntimes = Benchmark::benchmark(decompressFunction, runs, warmup, clearCache);
			size_t cSize = compressedSize(compressedColumn);
			size_t uSize = (sizeof(column) + sizeof(D) * column.size());
			return Benchmark::Result(compressRuntimes, decompressRuntimes, cSize, uSize);
		}
		else if (uniques.size() <= std::pow(2, 64))
		{
			std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^64" << std::endl;
			auto compressedColumn = compress<D, uint64_t>(column);
			std::function<std::pair<std::vector<D>, std::vector<uint64_t>> ()> compressFunction = [&column]() {
				return compress<D, uint64_t>(column);
			};
			std::function<std::vector<D> ()> decompressFunction = [&compressedColumn]() {
				return decompress(compressedColumn);
			};
			auto compressRuntimes = Benchmark::benchmark(compressFunction, runs, warmup, clearCache);
			auto decompressRuntimes = Benchmark::benchmark(decompressFunction, runs, warmup, clearCache);
			size_t cSize = compressedSize(compressedColumn);
			size_t uSize = (sizeof(column) + sizeof(D) * column.size());
			return Benchmark::Result(compressRuntimes, decompressRuntimes, cSize, uSize);
		}
		else {
			std::cout << "Cannot address more than 2^64 uniques" << std::endl;
		}
	}
}

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

	template <typename D>
	std::pair<std::map<D, std::vector<bool>>, std::vector<std::vector<bool>>> compress(const std::vector<D> &column) {
		std::map<D, std::vector<bool>> dictionary;
		std::vector<std::vector<bool>> attributeVector(column.size());

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
		buildCodes<D>(minHeap.top(), prefix, dictionary);
		for (int i = 0; i < column.size(); ++i)
		{
			attributeVector[i] = dictionary[column[i]];
		}
		return std::pair(dictionary, attributeVector);
	}

	template <typename D>
	std::vector<D> decompress(std::pair<std::map<D, std::vector<bool>>, std::vector<std::vector<bool>>> &compressed) {
		std::vector<D> decompressed(compressed.second.size());
		std::map<std::vector<bool>, D> reverseDictionary;
		for(auto const& [k, v] : compressed.first) {
			reverseDictionary[v] = k;
		}
		for (int i = 0; i < decompressed.size(); ++i)
		{
			decompressed[i] = reverseDictionary[compressed.second[i]];
		}
		return decompressed;
	}

	template <typename D>
	size_t compressedSize(std::pair<std::map<D, std::vector<bool>>, std::vector<std::vector<bool>>> &compressed) {
		size_t size = 0;
		// Size of data structures => map + std::vector
		size += sizeof(compressed.first) + sizeof(compressed.second);
		for(auto const& [k, v] : compressed.first) {
			// Size of dictionary => D + std::vector + n * bool (1 Byte)
			size += sizeof(k) + sizeof(v) * v.size();
		}
		// Size of attribute vector values
		for(auto attribute : compressed.second) {
			// std::vector + n * bool (1 Byte)
			size += sizeof(attribute) * attribute.size();
		}
		return size;
	}

	template <typename D>
	Benchmark::Result benchmark(const std::vector<D> &column, int runs, int warmup, bool clearCache) {
		std::cout << "Huffman - Compressing column" << std::endl;
		auto compressedColumn = compress<D>(column);
		std::function<std::pair<std::map<D, std::vector<bool>>, std::vector<std::vector<bool>>> ()> compressFunction = [&column]() {
			return compress<D>(column);
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
}


void fullDictionaryBenchmark(std::vector<std::vector<std::string>> &table, std::vector<std::string> &header,
	int runs, int warmup, bool clearCache,
	std::string cRatioFile, std::string cSizeFile, std::string uSizeFile, std::string cTimesFile, std::string dcTimesFile) {

	std::string dictionaryDirectory = "../data/dictionary/";

	std::vector<Benchmark::Result> results;
	for (int i = 0; i < header.size(); ++i)
	{
		std::cout << "Dictionary - Benchmarking column: " << header[i] << std::endl;
		auto benchmarkResult = Dictionary::benchmark<std::string>(table[i], runs, warmup, clearCache);
		results.push_back(benchmarkResult);
	}

	std::vector<double> cRatios;
	std::vector<size_t> cSizes;
	std::vector<size_t> uSizes;
	std::vector<std::vector<size_t>> cTimes;
	std::vector<std::vector<size_t>> dcTimes;
	for (int i = 0; i < results.size(); ++i)
	{
		cRatios.emplace_back(results[i].compressionRatio);
		cSizes.emplace_back(results[i].compressedSize);
		uSizes.emplace_back(results[i].uncompressedSize);	
		cTimes.emplace_back(results[i].compressionTimes);
		dcTimes.emplace_back(results[i].decompressionTimes);
	}
	CSV::writeLine<double>(header, cRatios, dictionaryDirectory + cRatioFile);
	CSV::writeLine<size_t>(header, cSizes, dictionaryDirectory + cSizeFile);
	CSV::writeLine<size_t>(header, uSizes, dictionaryDirectory + uSizeFile);
	CSV::writeMultiLine<size_t>(header, cTimes, dictionaryDirectory + cTimesFile);
	CSV::writeMultiLine<size_t>(header, dcTimes, dictionaryDirectory + dcTimesFile);
}


void fullHuffmanBenchmark(std::vector<std::vector<std::string>> &table, std::vector<std::string> &header,
	int runs, int warmup, bool clearCache,
	std::string cRatioFile, std::string cSizeFile, std::string uSizeFile, std::string cTimesFile, std::string dcTimesFile) {

	std::string dictionaryDirectory = "../data/huffman/";

	std::vector<Benchmark::Result> results;
	for (int i = 0; i < header.size(); ++i)
	{
		std::cout << "Huffman - Benchmarking column: " << header[i] << std::endl;
		auto benchmarkResult = Huffman::benchmark<std::string>(table[i], runs, warmup, clearCache);
		results.push_back(benchmarkResult);
	}

	std::vector<double> cRatios;
	std::vector<size_t> cSizes;
	std::vector<size_t> uSizes;
	std::vector<std::vector<size_t>> cTimes;
	std::vector<std::vector<size_t>> dcTimes;
	for (int i = 0; i < results.size(); ++i)
	{
		cRatios.emplace_back(results[i].compressionRatio);
		cSizes.emplace_back(results[i].compressedSize);
		uSizes.emplace_back(results[i].uncompressedSize);	
		cTimes.emplace_back(results[i].compressionTimes);
		dcTimes.emplace_back(results[i].decompressionTimes);
	}
	CSV::writeLine<double>(header, cRatios, dictionaryDirectory + cRatioFile);
	CSV::writeLine<size_t>(header, cSizes, dictionaryDirectory + cSizeFile);
	CSV::writeLine<size_t>(header, uSizes, dictionaryDirectory + uSizeFile);
	CSV::writeMultiLine<size_t>(header, cTimes, dictionaryDirectory + cTimesFile);
	CSV::writeMultiLine<size_t>(header, dcTimes, dictionaryDirectory + dcTimesFile);
}


int main(int argc, char* argv[]) {
	int runs = 100;
	int warmup = 10;
	bool clearCache = false;

	std::string dataFile = "../data/order.tbl";
	auto header = CSV::headerFromFile(dataFile);
	std::cout << "Loading table" << std::endl;
	auto start = std::chrono::system_clock::now();
	auto table = CSV::toColumnStore(dataFile);
	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	std::cout << "Finished loading in " << elapsed << " nanoseconds" << std::endl;
	std::cout << "Loaded " << table[0].size() << " lines" << std::endl;

	std::string cRatioFile = "compression_ratios.csv";
	std::string cSizeFile = "compressed_size.csv";
	std::string uSizeFile = "uncompressed_size.csv";
	std::string cTimesFile = "compression_times.csv";
	std::string dcTimesFile = "decompression_times.csv";


	// ------------------- Dictionary -------------- //

	fullDictionaryBenchmark(table, header, runs, warmup, clearCache, cRatioFile, cSizeFile, uSizeFile, cTimesFile, dcTimesFile);

	// ------------------- Huffman -------------- //

	fullHuffmanBenchmark(table, header, runs, warmup, clearCache, cRatioFile, cSizeFile, uSizeFile, cTimesFile, dcTimesFile);


	// gcc version 7.3.0 (MinGW-W64 project)
	// Command to compile code (Windows syntax!):
	// gcc .\main.cpp -lstdc++ -o main -std=c++1z; .\main.exe
}