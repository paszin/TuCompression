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
#include "csv.h"
#include "benchmark.cpp"
#include "dictionary.cpp"
#include "huffman.cpp"


template <typename C>
Benchmark::Result dictionaryBenchmarkColumn(int i, std::vector<std::string> &column, std::vector<std::string> &header,
        int runs, int warmup, bool clearCache) {
	std::vector<Benchmark::Result> results;
	std::cout << "Dictionary - Benchmarking column (" << i + 1 << "/" << header.size() << "): " << header[i] << std::endl;
	if (i == 0 || i == 1 || i == 7) {
		// Column to int
		std::vector<int> convertedColumn;
		std::transform(column.begin(), column.end(), std::back_inserter(convertedColumn), [](const std::string & str) { return std::stoi(str); });
		auto result = Dictionary::benchmark_with_dtype<int, C>(convertedColumn, runs, warmup, clearCache);
		// TODO: Run aggregate tests
		// {
		// 	where_copy_op
		// 	std::function<bool (int)> predicate = [](int i) {
		// 		return i > 5;
		// 	};
		// 	auto func = [predicate](std::pair<std::vector<int>, std::vector<C>> &col) -> std::vector<int> {
		// 		return Dictionary::where_copy_op(col, predicate);
		// 	};
		// 	auto runtimes = Dictionary::benchmark_op_with_dtype<int, C, std::vector<int>>(column, 1, 1, false, func);
		// 	result.aggregateRuntimes.push_back(runtimes);
		// 	result.aggregateNames.push_back("where_copy_gt5_");
		// }
		return result;
	}
	else if (i == 3) {
		// Column to float
		std::vector<float> convertedColumn;
		std::transform(column.begin(), column.end(), std::back_inserter(convertedColumn), [](const std::string & str) { return std::stof(str); });
		return Dictionary::benchmark_with_dtype<float, C>(convertedColumn, runs, warmup, clearCache);
	}
	else {
		// Column as string
		return Dictionary::benchmark_with_dtype<std::string, C>(column, runs, warmup, clearCache);
	}
}


void fullDictionaryBenchmark(std::vector<std::vector<std::string>> &table, std::vector<std::string> &header,
                             int runs, int warmup, bool clearCache,
                             std::string cRatioFile, std::string cSizeFile, std::string uSizeFile, std::string cTimesFile, std::string dcTimesFile) {

	std::string dataDirectory = "../data/dictionary/";

	std::vector<Benchmark::Result> results;
	for (int i = 0; i < header.size(); ++i)
	{
		std::set<std::string> uniques;
		auto column = table[i];
		for (auto cell : column) {
			uniques.emplace(cell);
		}
		if (uniques.size() <= std::pow(2, 8)) {
			std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^8" << std::endl;
			results.push_back(dictionaryBenchmarkColumn<uint8_t>(i, column, header, runs, warmup, clearCache));
		}
		else if (uniques.size() <= std::pow(2, 16))
		{
			std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^16" << std::endl;
			results.push_back(dictionaryBenchmarkColumn<uint16_t>(i, column, header, runs, warmup, clearCache));
		}
		else if (uniques.size() <= std::pow(2, 32))
		{
			std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^32" << std::endl;
			results.push_back(dictionaryBenchmarkColumn<uint32_t>(i, column, header, runs, warmup, clearCache));
		}
		else if (uniques.size() <= std::pow(2, 64))
		{
			std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^64" << std::endl;
			results.push_back(dictionaryBenchmarkColumn<uint64_t>(i, column, header, runs, warmup, clearCache));
		}
		else {
			std::cout << "Cannot address more than 2^64 uniques" << std::endl;
		}
	}

	std::cout << "Dictionary - Finished" << std::endl;
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
	CSV::writeLine<double>(header, cRatios, dataDirectory + cRatioFile);
	CSV::writeLine<size_t>(header, cSizes, dataDirectory + cSizeFile);
	CSV::writeLine<size_t>(header, uSizes, dataDirectory + uSizeFile);
	CSV::writeMultiLine<size_t>(header, cTimes, dataDirectory + cTimesFile);
	CSV::writeMultiLine<size_t>(header, dcTimes, dataDirectory + dcTimesFile);
	for (int j = 0; j < results.size(); ++j) {
		for (int i = 0; i < results[j].aggregateRuntimes.size(); ++i)
		{
			auto times = results[j].aggregateRuntimes[i];
			auto name = results[j].aggregateNames[i];
			CSV::writeLine<size_t>(header, times, dataDirectory + header[j] + name + ".csv");
		}
	}
}


void fullHuffmanBenchmark(std::vector<std::vector<std::string>> &table, std::vector<std::string> &header,
                          int runs, int warmup, bool clearCache,
                          std::string cRatioFile, std::string cSizeFile, std::string uSizeFile, std::string cTimesFile, std::string dcTimesFile) {

	std::string dataDirectory = "../data/huffman/";

	std::vector<Benchmark::Result> results;
	for (int i = 0; i < header.size(); ++i)
	{
		std::cout << "Huffman - Benchmarking column (" << i + 1 << "/" << header.size() << "): " << header[i] << std::endl;
		if (i == 0 || i == 1 || i == 7) {
			// Column to int
			std::vector<int> convertedColumn;
			std::transform(table[i].begin(), table[i].end(), std::back_inserter(convertedColumn), [](const std::string & str) { return std::stoi(str); });
			auto benchmarkResult = Huffman::benchmark(convertedColumn, runs, warmup, clearCache);
			// TODO: Implement aggregates on huffman
			// TODO: Run aggregate tests
			// {
			// 	where_copy_op
			// 	std::function<bool (int)> predicate = [](int i) {
			// 		return i > 5;
			// 	};
			// 	auto func = [predicate](std::pair<std::vector<int>, std::vector<C>> &col) -> std::vector<int> {
			// 		return Dictionary::where_copy_op(col, predicate);
			// 	};
			// 	auto runtimes = Dictionary::benchmark_op_with_dtype<int, C, std::vector<int>>(column, 1, 1, false, func);
			// 	result.aggregateRuntimes.push_back(runtimes);
			// 	result.aggregateNames.push_back("where_copy_gt5_");
			// }
			results.push_back(benchmarkResult);
		}
		else if (i == 3) {
			// Column to float
			std::vector<float> convertedColumn;
			std::transform(table[i].begin(), table[i].end(), std::back_inserter(convertedColumn), [](const std::string & str) { return std::stof(str); });
			auto benchmarkResult = Huffman::benchmark(convertedColumn, runs, warmup, clearCache);
			results.push_back(benchmarkResult);
		}
		else {
			// Column as string
			auto benchmarkResult = Huffman::benchmark(table[i], runs, warmup, clearCache);
			results.push_back(benchmarkResult);
		}
	}

	std::cout << "Huffman - Finished" << std::endl;
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
	CSV::writeLine<double>(header, cRatios, dataDirectory + cRatioFile);
	CSV::writeLine<size_t>(header, cSizes, dataDirectory + cSizeFile);
	CSV::writeLine<size_t>(header, uSizes, dataDirectory + uSizeFile);
	CSV::writeMultiLine<size_t>(header, cTimes, dataDirectory + cTimesFile);
	CSV::writeMultiLine<size_t>(header, dcTimes, dataDirectory + dcTimesFile);
	for (int j = 0; j < results.size(); ++j) {
		for (int i = 0; i < results[j].aggregateRuntimes.size(); ++i)
		{
			auto times = results[j].aggregateRuntimes[i];
			auto name = results[j].aggregateNames[i];
			CSV::writeLine<size_t>(header, times, dataDirectory + header[j] + name + ".csv");
		}
	}
}


int main(int argc, char* argv[]) {
	int runs = 10;
	int warmup = 1;
	bool clearCache = false;

	// ------------------- Parse Flags -------------- //
	std::vector<std::string> args;
	for (int i = 1; i < argc; ++i)
	{
		args.push_back(argv[i]);
	}
	bool dictionary = false;
	bool huffman = false;
	if (argc < 2) {
		dictionary = true;
		huffman = true;
	}
	else {
		for (auto arg : args) {
			if (arg == "-dictionary") {
				dictionary = true;
			}
			else if (arg == "-huffman") {
				huffman = true;
			}
			else {
				std::cerr << arg << " is an unrecognised flag.\nThe following flags are allowed:\n\t-dictionary\n\t-huffman\n\tor no flag to run all" << std::endl;
				return 1;
			}
		}
	}

	// ------------------- Load Table -------------- //
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
	if (dictionary) {
		fullDictionaryBenchmark(table, header, runs, warmup, clearCache, cRatioFile, cSizeFile, uSizeFile, cTimesFile, dcTimesFile);
	}
	if (huffman) {
		fullHuffmanBenchmark(table, header, runs, warmup, clearCache, cRatioFile, cSizeFile, uSizeFile, cTimesFile, dcTimesFile);
	}

	// gcc version 7.3.0

	// With optmizations
	// gcc main.cpp -lstdc++ -std=c++1z -O2 -o main

	// Without optimizations (we have to link math with -lm)
	// gcc main.cpp -lstdc++ -std=c++1z -lm -o main
}