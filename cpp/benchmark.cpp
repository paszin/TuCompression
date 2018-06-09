namespace Benchmark
{
template <typename C>
std::vector<size_t> benchmark(std::function<C ()> wrapperFunction, int runs, int warmup, bool clearCache) {
	if (clearCache == false)
	{
		for (int i = 0; i < warmup; ++i)
		{
			std::cout << "\rBenchmarking - Warmup: " << i << std::flush;
			volatile auto compressed = wrapperFunction();
		}
		std::cout << std::endl;
	}
	auto runTimes = std::vector<size_t>(runs);
	std::cout << "Beginning benchmark" << std::endl;
	for (int i = 0; i < runs; ++i)
	{
		std::cout << "\rBenchmarking - Run: " << i << std::flush;
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
	std::vector<std::vector<size_t>> aggregateRuntimes;
	std::vector<std::string> aggregateNames;

	Result(std::vector<size_t> cT, std::vector<size_t> dT, size_t cSize, size_t uSize) :
		compressionTimes(cT), decompressionTimes(dT), compressedSize(cSize), uncompressedSize(uSize),
		compressionRatio((double)uSize / (double)cSize) {}
};
} // end namespace Benchmark