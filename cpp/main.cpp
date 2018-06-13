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
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include "csv.h"
#include "benchmark.cpp"
#include "dictionary.cpp"
#include "huffman.cpp"


template <typename C>
std::pair<Benchmark::CompressionResult, Benchmark::OpResult> dictionaryBenchmarkColumn(int i, std::vector<std::string> &column, std::vector<std::string> &header,
        int runs, int warmup, bool clearCache, bool compress, bool op) {
	std::cout << "Dictionary - Benchmarking column (" << i + 1 << "/" << header.size() << "): " << header[i] << std::endl;
	Benchmark::CompressionResult compressionResult;
	Benchmark::OpResult opResult;
	if (i == 0 || i == 1 || i == 7) {
		// Column to int
		std::vector<int> convertedColumn;
		std::transform(column.begin(), column.end(), std::back_inserter(convertedColumn), [](const std::string & str) { return std::stoi(str); });
		if (compress) {
			compressionResult = Dictionary::benchmark_with_dtype<int, C>(convertedColumn, runs, warmup, clearCache);
		}
		if (op) {
			auto compressedColumn = Dictionary::compress<int, C>(convertedColumn);
			if (i == 7) {
				// SHIPPRIORITY
				{
					auto func = [](std::pair<std::vector<int>, std::vector<C>> &col) -> size_t {
						return Dictionary::sum_op(col);
					};
					auto runtimes = Dictionary::benchmark_op_with_dtype<int, C, size_t>(compressedColumn, runs, warmup, clearCache, func);
					opResult.aggregateRuntimes.push_back(runtimes);
					opResult.aggregateNames.push_back("sum");
				}
			}
		}
	}
	if (i == 4) {
		// Column to std::time_t
		// ORDERDATE
		std::vector<std::time_t> convertedColumn;
		auto transform_fn = [](const std::string & str) {
			std::tm t = {};
			std::istringstream ss(str);
			ss >> std::get_time(&t, "%Y-%m-%d");
			if (ss.fail()) {
				throw std::invalid_argument("Cannot convert " + str + " to time");
			}
			return std::mktime(&t);
		};
		std::transform(column.begin(), column.end(), std::back_inserter(convertedColumn), transform_fn);
		if (compress) {
			compressionResult = Dictionary::benchmark_with_dtype<std::time_t, C>(convertedColumn, runs, warmup, clearCache);
		}
		if (op) {
			auto compressedColumn = Dictionary::compress<std::time_t, C>(convertedColumn);
			{
				// 1996-01-02
				std::tm date;
				date.tm_mon = 0;
				date.tm_sec = 0;
				date.tm_min = 0;
				date.tm_hour = 0;
				date.tm_wday = 0;
				date.tm_yday = 0;
				date.tm_year = 96;
				date.tm_mday = 2;
				std::function<bool (std::time_t)> predicate = [&date](std::time_t i) {
					// 1996-01-02
					return i < std::mktime(&date);
				};
				auto func = [predicate](std::pair<std::vector<std::time_t>, std::vector<C>> &col) -> std::vector<std::time_t> {
					return Dictionary::where_view_op(col, predicate);
				};
				auto runtimes = Dictionary::benchmark_op_with_dtype<std::time_t, C, std::vector<std::time_t>>(compressedColumn, runs, warmup, clearCache, func);
				opResult.aggregateRuntimes.push_back(runtimes);
				opResult.aggregateNames.push_back("where_view_less_1996-01-02");
			}
			{
				// 1996-01-02
				std::tm date;
				date.tm_mon = 0;
				date.tm_sec = 0;
				date.tm_min = 0;
				date.tm_hour = 0;
				date.tm_wday = 0;
				date.tm_yday = 0;
				date.tm_year = 96;
				date.tm_mday = 2;
				std::function<bool (std::time_t)> predicate = [&date](std::time_t i) {
					// 1996-01-02
					return i < std::mktime(&date);
				};
				auto func = [predicate](std::pair<std::vector<std::time_t>, std::vector<C>> &col) -> std::vector<std::time_t> {
					return Dictionary::where_copy_op(col, predicate);
				};
				auto runtimes = Dictionary::benchmark_op_with_dtype<std::time_t, C, std::vector<std::time_t>>(compressedColumn, runs, warmup, clearCache, func);
				opResult.aggregateRuntimes.push_back(runtimes);
				opResult.aggregateNames.push_back("where_copy_less_1996-01-02");
			}
		}
	}
	else if (i == 3) {
		// Column to float
		// TOTALPRICE
		std::vector<float> convertedColumn;
		std::transform(column.begin(), column.end(), std::back_inserter(convertedColumn), [](const std::string & str) { return std::stof(str); });
		if (compress) {
			compressionResult = Dictionary::benchmark_with_dtype<float, C>(convertedColumn, runs, warmup, clearCache);
		}
		if (op) {
			auto compressedColumn = Dictionary::compress<float, C>(convertedColumn);
			{
				auto func = [](std::pair<std::vector<float>, std::vector<C>> &col) -> float {
					return Dictionary::min_op(col);
				};
				auto runtimes = Dictionary::benchmark_op_with_dtype<float, C, float>(compressedColumn, runs, warmup, clearCache, func);
				opResult.aggregateRuntimes.push_back(runtimes);
				opResult.aggregateNames.push_back("min");
			}
			{
				auto func = [](std::pair<std::vector<float>, std::vector<C>> &col) -> float {
					return Dictionary::max_op(col);
				};
				auto runtimes = Dictionary::benchmark_op_with_dtype<float, C, float>(compressedColumn, runs, warmup, clearCache, func);
				opResult.aggregateRuntimes.push_back(runtimes);
				opResult.aggregateNames.push_back("max");
			}
			{
				auto func = [](std::pair<std::vector<float>, std::vector<C>> &col) -> float {
					return Dictionary::avg_op(col);
				};
				auto runtimes = Dictionary::benchmark_op_with_dtype<float, C, float>(compressedColumn, runs, warmup, clearCache, func);
				opResult.aggregateRuntimes.push_back(runtimes);
				opResult.aggregateNames.push_back("avg");
			}
			{
				auto func = [](std::pair<std::vector<float>, std::vector<C>> &col) -> float {
					return Dictionary::sum_op(col);
				};
				auto runtimes = Dictionary::benchmark_op_with_dtype<float, C, float>(compressedColumn, runs, warmup, clearCache, func);
				opResult.aggregateRuntimes.push_back(runtimes);
				opResult.aggregateNames.push_back("sum");
			}
		}
	}
	else {
		// Column as string
		if (compress) {
			compressionResult = Dictionary::benchmark_with_dtype<std::string, C>(column, runs, warmup, clearCache);
		}
		if (op) {
			auto compressedColumn = Dictionary::compress<std::string, C>(column);
			if (i == 2) {
				// ORDERSTATUS
				{
					std::function<bool (std::string)> predicate = [](std::string i) {
						return i == "O";
					};
					auto func = [predicate](std::pair<std::vector<std::string>, std::vector<C>> &col) -> size_t {
						return Dictionary::count_where_op(col, predicate);
					};
					auto runtimes = Dictionary::benchmark_op_with_dtype<std::string, C, size_t>(compressedColumn, runs, warmup, clearCache, func);
					opResult.aggregateRuntimes.push_back(runtimes);
					opResult.aggregateNames.push_back("count_where_equals_O");
				}
				{
					std::function<bool (std::string)> predicate = [](std::string i) {
						return i == "P";
					};
					auto func = [predicate](std::pair<std::vector<std::string>, std::vector<C>> &col) -> size_t {
						return Dictionary::count_where_op(col, predicate);
					};
					auto runtimes = Dictionary::benchmark_op_with_dtype<std::string, C, size_t>(compressedColumn, runs, warmup, clearCache, func);
					opResult.aggregateRuntimes.push_back(runtimes);
					opResult.aggregateNames.push_back("count_where_equals_P");
				}
			}
		}
	}
	return std::pair(compressionResult, opResult);
}


void fullDictionaryBenchmark(std::vector<std::vector<std::string>> &table, std::vector<std::string> &header,
                             int runs, int warmup, bool clearCache, bool compress, bool op,
                             std::string cRatioFile, std::string cSizeFile, std::string uSizeFile, std::string cTimesFile, std::string dcTimesFile) {

	std::string dataDirectory = "../data/dictionary/";

	std::vector<std::pair<Benchmark::CompressionResult, Benchmark::OpResult>> results;
	for (int i = 0; i < header.size(); ++i)
	{
		std::set<std::string> uniques;
		auto column = table[i];
		for (auto cell : column) {
			uniques.emplace(cell);
		}
		if (uniques.size() <= std::pow(2, 8)) {
			std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^8" << std::endl;
			results.push_back(dictionaryBenchmarkColumn<uint8_t>(i, column, header, runs, warmup, clearCache, compress, op));
		}
		else if (uniques.size() <= std::pow(2, 16))
		{
			std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^16" << std::endl;
			results.push_back(dictionaryBenchmarkColumn<uint16_t>(i, column, header, runs, warmup, clearCache, compress, op));
		}
		else if (uniques.size() <= std::pow(2, 32))
		{
			std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^32" << std::endl;
			results.push_back(dictionaryBenchmarkColumn<uint32_t>(i, column, header, runs, warmup, clearCache, compress, op));
		}
		else if (uniques.size() <= std::pow(2, 64))
		{
			std::cout << "Dictionary - Compressing column - " << uniques.size() << " = 2^64" << std::endl;
			results.push_back(dictionaryBenchmarkColumn<uint64_t>(i, column, header, runs, warmup, clearCache, compress, op));
		}
		else {
			std::cout << "Cannot address more than 2^64 uniques" << std::endl;
		}
	}

	std::cout << "Dictionary - Finished" << std::endl;
	if (compress) {
		std::vector<double> cRatios;
		std::vector<size_t> cSizes;
		std::vector<size_t> uSizes;
		std::vector<std::vector<size_t>> cTimes;
		std::vector<std::vector<size_t>> dcTimes;
		for (int i = 0; i < results.size(); ++i)
		{
			auto result = results[i].first;
			cRatios.emplace_back(result.compressionRatio);
			cSizes.emplace_back(result.compressedSize);
			uSizes.emplace_back(result.uncompressedSize);
			cTimes.emplace_back(result.compressionTimes);
			dcTimes.emplace_back(result.decompressionTimes);
		}
		CSV::writeLine<double>(header, cRatios, dataDirectory + cRatioFile);
		CSV::writeLine<size_t>(header, cSizes, dataDirectory + cSizeFile);
		CSV::writeLine<size_t>(header, uSizes, dataDirectory + uSizeFile);
		CSV::writeMultiLine<size_t>(header, cTimes, dataDirectory + cTimesFile);
		CSV::writeMultiLine<size_t>(header, dcTimes, dataDirectory + dcTimesFile);
	}
	if (op) {
		for (int j = 0; j < results.size(); ++j) {
			auto result = results[j].second;
			for (int i = 0; i < result.aggregateRuntimes.size(); ++i)
			{
				CSV::writeSingleColumn<size_t>(header[j], result.aggregateRuntimes[i], dataDirectory + "AGG__" + header[j] + "__" + result.aggregateNames[i] + ".csv");
			}
		}
	}
}


void fullHuffmanBenchmark(std::vector<std::vector<std::string>> &table, std::vector<std::string> &header,
                          int runs, int warmup, bool clearCache,
                          std::string cRatioFile, std::string cSizeFile, std::string uSizeFile, std::string cTimesFile, std::string dcTimesFile) {

	std::string dataDirectory = "../data/huffman/";

	std::vector<Benchmark::CompressionResult> results;
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
	// for (int j = 0; j < results.size(); ++j) {
	// 	for (int i = 0; i < results[j].aggregateRuntimes.size(); ++i)
	// 	{
	// 		auto times = results[j].aggregateRuntimes[i];
	// 		auto name = results[j].aggregateNames[i];
	// 		CSV::writeLine<size_t>(header, times, dataDirectory + "AGG__" + header[j] + "__" + name + ".csv");
	// 	}
	// }
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
	bool compress = false;
	bool op = false;
	for (auto arg : args) {
		if (arg == "-dictionary") {
			std::cout << "Enabled: dictionary" << std::endl;
			dictionary = true;
		}
		else if (arg == "-huffman") {
			std::cout << "Enabled: huffman" << std::endl;
			huffman = true;
		}
		else if (arg == "-compress") {
			std::cout << "Enabled: compress" << std::endl;
			compress = true;
		}
		else if (arg == "-op") {
			std::cout << "Enabled: op" << std::endl;
			op = true;
		}
		else {
			std::cerr << arg << " is an unrecognised flag.\nThe following flags are allowed:\n\t-dictionary\n\t-huffman\n\t-compress (enables compression benchmarks)\n\t-op (enables operation benchmarks)\n\tor no flag of either pairs to enable both" << std::endl;
			return 1;
		}
	}
	if (compress == false && op == false) {
		std::cout << "Enabled: compress" << std::endl;
		compress = true;
		std::cout << "Enabled: op" << std::endl;
		op = true;
	}
	if (dictionary == false && huffman == false) {
		std::cout << "Enabled: dictionary" << std::endl;
		dictionary = true;
		std::cout << "Enabled: huffman" << std::endl;
		huffman = true;
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
	try {
		if (dictionary) {
			fullDictionaryBenchmark(table, header, runs, warmup, clearCache, compress, op, cRatioFile, cSizeFile, uSizeFile, cTimesFile, dcTimesFile);
		}
		if (huffman) {
			fullHuffmanBenchmark(table, header, runs, warmup, clearCache, cRatioFile, cSizeFile, uSizeFile, cTimesFile, dcTimesFile);
		}
	} catch (const std::invalid_argument& e) {
		return 0;
	}
	return 1;

	// gcc version 7.3.0

	// With optmizations
	// gcc main.cpp -lstdc++ -std=c++1z -O2 -o main

	// Without optimizations (we have to link math with -lm)
	// gcc main.cpp -lstdc++ -std=c++1z -lm -o main
}