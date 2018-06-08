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


void fullDictionaryBenchmark(std::vector<std::vector<std::string>> &table, std::vector<std::string> &header,
                             int runs, int warmup, bool clearCache,
                             std::string cRatioFile, std::string cSizeFile, std::string uSizeFile, std::string cTimesFile, std::string dcTimesFile) {

	std::string dictionaryDirectory = "../data/dictionary/";

	std::vector<Benchmark::Result> results;
	for (int i = 0; i < header.size(); ++i)
	{
		std::cout << "Dictionary - Benchmarking column (" << i + 1 << "/" << header.size() << "): " << header[i] << std::endl;
		if (i == 0 || i == 1 || i == 7) {
			// Column to int
			std::vector<int> convertedColumn;
			std::transform(table[i].begin(), table[i].end(), std::back_inserter(convertedColumn), [](const std::string & str) { return std::stoi(str); });
			auto benchmarkResult = Dictionary::benchmark(convertedColumn, runs, warmup, clearCache);
			results.push_back(benchmarkResult);
		}
		else if (i == 3) {
			// Column to float
			std::vector<float> convertedColumn;
			std::transform(table[i].begin(), table[i].end(), std::back_inserter(convertedColumn), [](const std::string & str) { return std::stof(str); });
			auto benchmarkResult = Dictionary::benchmark(convertedColumn, runs, warmup, clearCache);
			results.push_back(benchmarkResult);
		}
		else {
			// Column as string
			auto benchmarkResult = Dictionary::benchmark(table[i], runs, warmup, clearCache);
			results.push_back(benchmarkResult);
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
		std::cout << "Huffman - Benchmarking column (" << i + 1 << "/" << header.size() << "): " << header[i] << std::endl;
		if (i == 0 || i == 1 || i == 7) {
			// Column to int
			std::vector<int> convertedColumn;
			std::transform(table[i].begin(), table[i].end(), std::back_inserter(convertedColumn), [](const std::string & str) { return std::stoi(str); });
			auto benchmarkResult = Huffman::benchmark(convertedColumn, runs, warmup, clearCache);
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
	CSV::writeLine<double>(header, cRatios, dictionaryDirectory + cRatioFile);
	CSV::writeLine<size_t>(header, cSizes, dictionaryDirectory + cSizeFile);
	CSV::writeLine<size_t>(header, uSizes, dictionaryDirectory + uSizeFile);
	CSV::writeMultiLine<size_t>(header, cTimes, dictionaryDirectory + cTimesFile);
	CSV::writeMultiLine<size_t>(header, dcTimes, dictionaryDirectory + dcTimesFile);
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