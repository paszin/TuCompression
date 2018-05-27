#include <string>
#include <chrono>
#include <functional>
#include <numeric>
#include "csv.h"
#include "compressed.h"
#include "dictionary.h"
// #include "huffman.h"
// #include "cache.cpp"


template <typename T>
class ColumnBenchmark
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
			if (clearCache == true)
			{
				// TODO: call mem_flush. Do it like this?
				// mem_flush(&column, sizeof(column));
			}
			auto start = std::chrono::system_clock::now();
			volatile auto compressed = wrapperFunction();
			auto end = std::chrono::system_clock::now();
			runTimes[i] = std::chrono::duration_cast<T>(end - start).count();
		}
		return runTimes;
	}

public:
	template <typename D, typename A>
	std::vector<size_t> compressBenchmark(Encoder<D,A> *encoder, const std::vector<D> &column, int runs, int warmup, bool clearCache) {
		auto wrapperFunction = [&column, encoder]() {
			return encoder->compress(column);
		};
		return benchmark<CompressedColumn<D, A>>(wrapperFunction, runs, warmup, clearCache);
	}

	template <typename D, typename A>
	std::vector<size_t> decompressBenchmark(Encoder<D,A> *encoder, const CompressedColumn<D,A> &column, int runs, int warmup, bool clearCache) {
		auto wrapperFunction = [&column, encoder]() {
			return encoder->decompress(column);
		};
		return benchmark<std::vector<D>>(wrapperFunction, runs, warmup, clearCache);
	}
};

void dictionaryBenchmark(const std::vector<std::string> &header, const std::vector<std::vector<std::string>> &table,
						 int runs, int warmup=10, bool clearCache=false) {
	DictionaryEncoder<std::string, size_t> encoder;
	ColumnBenchmark<std::chrono::nanoseconds> benchmark;

	// Compress
	std::vector<CompressedColumn<std::string, size_t>> compressedTable;
	for (int i = 0; i < table.size(); ++i)
	{
		compressedTable.push_back(encoder.compress(table[i]));
	}

	// Benchmark columns
	std::vector<std::vector<size_t>> compressionRuntimes(table.size());
	std::vector<std::vector<size_t>> decompressionRuntimes(table.size());
	for (int i = 0; i < table.size(); ++i)
	{
		compressionRuntimes[i] = benchmark.compressBenchmark(&encoder, table[i], runs, warmup, clearCache);
		auto compressionAverage = std::accumulate(compressionRuntimes[i].begin(), compressionRuntimes[i].end(), 0.0)/compressionRuntimes[i].size();
		std::cout << "Dictionary Compression: Average Run Time for " << header[i] << ": " << compressionAverage << std::endl;
		
		decompressionRuntimes[i] = benchmark.decompressBenchmark(&encoder, compressedTable[i], runs, warmup, clearCache);
		auto decompressionAverage = std::accumulate(decompressionRuntimes[i].begin(), decompressionRuntimes[i].end(), 0.0)/decompressionRuntimes[i].size();
		std::cout << "Dictionary Decompression: Average Run Time for " << header[i] << ": " << decompressionAverage << std::endl;
	}
}


int main(int argc, char* argv[]) {
	std::string dataFile = "../data/order.short.tbl";
	auto header = CSV::headerFromFile(dataFile);
	std::cout << "Loading table";
	auto start = std::chrono::system_clock::now();
	auto table = CSV::toColumnStore(dataFile);
	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
	std::cout << "\rFinished loading in " << elapsed << " seconds" << std::endl;	

	std::cout << "Benchmarking Dictionary Encoding" << std::endl;
	dictionaryBenchmark(header, table, 100);

	std::cout << "Benchmarking Huffman Encoding" << std::endl;
	// huffmanBenchmark(header, table);


	// gcc version 7.3.0 (MinGW-W64 project)
	// Command to compile code (Windows syntax!):
	// gcc .\csv.cpp .\compression.cpp -lstdc++ -o compression -std=c++1z; .\compression.exe
}
