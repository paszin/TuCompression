#include <string>
#include <chrono>
#include <functional>
#include <numeric>
#include "csv.h"
#include "compressed.h"
#include "dictionary.h"
#include "huffman.h"
#include "cache.cpp"


/**
	Runs a de/compression function 'runs' times on 'column'. Performs a warmup for 'warmup' times or
	clears the cache after every run if 'clearCache' is true.
	Returns the elapsed time for each run.
*/
template <typename T, typename O, typename I>
std::vector<size_t> benchmark(std::function<O(I)> deAndCompress, int runs, I column, int warmup=10, bool clearCache=false) {
	if (clearCache == false)
	{
		for (int i = 0; i < warmup; ++i)
		{
			volatile auto compressed = deAndCompress(column);
		}
	}
	auto runTimes = std::vector<size_t>(runs);
	for (int i = 0; i < runs; ++i)
	{
		if (clearCache == true)
		{
			// TODO: call mem_flush
		}
		auto start = std::chrono::system_clock::now();
		volatile auto compressed = deAndCompress(column);
		auto end = std::chrono::system_clock::now();
		runTimes[i] = std::chrono::duration_cast<T>(end - start).count();
	}

	return runTimes;
}

// NOTE: I know the *Benchmark functions are duplicate code, but I don't see a use in refactoring everything to polymorphism.

void dictionaryBenchmark(std::vector<std::string> header, std::vector<std::vector<std::string>> table) {
	// Compress
	std::vector<CompressedColumn> compressedTable;
	for (int i = 0; i < table.size(); ++i)
	{
		auto column = table[i];
		auto runTimes = benchmark<std::chrono::nanoseconds, CompressedColumn, std::vector<std::string>>(Dictionary::compress, 100, column);
		auto average = std::accumulate(runTimes.begin(), runTimes.end(), 0.0)/runTimes.size();
		std::cout << "Dictionary Compression: Average Run Time for " << header[i] << ": " << average << std::endl;
		auto compressedColumn = Dictionary::compress(column);
		compressedTable.push_back(compressedColumn);
	}

	// Decompress
	std::vector<std::vector<std::string>> decompressedTable(compressedTable.size());
	for (int i = 0; i < compressedTable.size(); ++i)
	{
		auto column = compressedTable[i];
		auto runTimes = benchmark<std::chrono::nanoseconds, std::vector<std::string>, CompressedColumn>(Dictionary::decompress, 100, column);
		auto average = std::accumulate(runTimes.begin(), runTimes.end(), 0.0)/runTimes.size();
		std::cout << "Dictionary Decompression: Average Run Time for " << header[i] << ": " << average << std::endl;
		auto decompressedColumn = Dictionary::decompress(column);
		decompressedTable.push_back(decompressedColumn);
	}
}

void huffmanBenchmark(std::vector<std::string> header, std::vector<std::vector<std::string>> table) {
	// Compress
	std::vector<CompressedColumn> compressedTable;
	for (int i = 0; i < table.size(); ++i)
	{
		auto column = table[i];
		auto runTimes = benchmark<std::chrono::nanoseconds, CompressedColumn, std::vector<std::string>>(Huffman::compress, 100, column);
		auto average = std::accumulate(runTimes.begin(), runTimes.end(), 0.0)/runTimes.size();
		std::cout << "Huffman Compression: Average Run Time for " << header[i] << ": " << average << std::endl;
		auto compressedColumn = Huffman::compress(column);
		compressedTable.push_back(compressedColumn);
	}

	// Decompress
	std::vector<std::vector<std::string>> decompressedTable(compressedTable.size());
	for (int i = 0; i < compressedTable.size(); ++i)
	{
		auto column = compressedTable[i];
		auto runTimes = benchmark<std::chrono::nanoseconds, std::vector<std::string>, CompressedColumn>(Huffman::decompress, 100, column);
		auto average = std::accumulate(runTimes.begin(), runTimes.end(), 0.0)/runTimes.size();
		std::cout << "Huffman Decompression: Average Run Time for " << header[i] << ": " << average << std::endl;
		auto decompressedColumn = Huffman::decompress(column);
		decompressedTable.push_back(decompressedColumn);
	}
}

int main(int argc, char* argv[]) {
	std::string dataFile = "../data/order.short.tbl";
	auto header = CSV::headerFromFile(dataFile);
	auto table = CSV::toColumnStore(dataFile);

	dictionaryBenchmark(header, table);

	huffmanBenchmark(header, table);


	// gcc version 7.3.0 (MinGW-W64 project)
	// Command to compile code (Windows syntax!):
	// gcc .\compression.cpp .\csv.cpp .\dictionary.cpp .\huffman.cpp -lstdc++ -o compression; .\compression.exe
}
