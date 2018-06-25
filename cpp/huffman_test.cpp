#include <stddef.h>
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
#include "benchmark.cpp"
#include "huffman.cpp"

int main(int argc, char const *argv[])
{
	std::bitset<64> bs(pow(2, 60));
	assert(Huffman::getCodeLength(bs) == 4);
	{
		std::vector<int> column = {1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 5, 6};
		for(size_t i = 0; i < 40; i++)
		{
			column.push_back(i);
		}
		auto compressedColumn = Huffman::compress<int, 64>(column);
		auto compressedPair = std::make_pair(std::get<0>(compressedColumn), std::get<1>(compressedColumn));
		auto decompressed = Huffman::decompress(compressedPair);
		assert(column == decompressed);

		auto dictionary = std::get<0>(compressedColumn);
		auto attributeVector = std::get<1>(compressedColumn);
		auto bounds = std::get<2>(compressedColumn);


		size_t count = Huffman::count_where_op_equal<int, 64>(std::get<0>(compressedColumn), std::get<1>(compressedColumn), std::get<2>(compressedColumn), 1);
		assert(count == 5);

		count = Huffman::count_where_op_range<int, 64>(std::get<0>(compressedColumn), std::get<1>(compressedColumn), std::get<2>(compressedColumn), 3, 6);
		std::cout << "Count (7): " << count << '\n';
		assert(count == 7);

		count = Huffman::count_where_op_range<int, 64>(std::get<0>(compressedColumn), std::get<1>(compressedColumn), std::get<2>(compressedColumn), 30, NULL);
		std::cout << "Count (10): " << count << '\n';
		assert(count == 10);

		count = Huffman::count_where_op_range<int, 64>(std::get<0>(compressedColumn), std::get<1>(compressedColumn), std::get<2>(compressedColumn), NULL, 2);
		std::cout << "Count (6): " << count << '\n';
		assert(count == 6);

		int min = Huffman::min_op<int>(std::get<2>(compressedColumn));
		std::cout << "Min: " << min << '\n';
		assert(min == 0);

		int max = Huffman::max_op<int>(std::get<2>(compressedColumn));
		std::cout << "Max: " << max << '\n';
		assert(max == 39);

		int sum = Huffman::sum_where_op_range<int, 64>(std::get<0>(compressedColumn), std::get<1>(compressedColumn), std::get<2>(compressedColumn), 10, 11);
		std::cout << "Sum (10): " << sum << '\n';
		assert(sum == 10);

		sum = Huffman::sum_where_op_range<int, 64>(std::get<0>(compressedColumn), std::get<1>(compressedColumn), std::get<2>(compressedColumn), NULL, NULL);
		std::cout << "Sum (811): " << sum << '\n';
		assert(sum == 811);

		float avg = Huffman::avg_op<int, 64>(std::get<0>(compressedColumn), std::get<1>(compressedColumn));
		std::cout << "AVG (15.596): " << avg << '\n';
		assert(avg > 15.596 && avg < 15.597);

		std::vector<int> values = Huffman::values_where_range_op<int, 64>(std::get<0>(compressedColumn), std::get<1>(compressedColumn),std::get<2>(compressedColumn), 30, 32);
		std::cout << "Values (30, 31)" << values[0] << ", " << values[1] << '\n';
		assert(values.size() ==2);

		values = Huffman::indexes_where_range_op<int, 64>(std::get<0>(compressedColumn), std::get<1>(compressedColumn), 30, 32);
		std::cout << "Values Indexes (42, 43)" << values[0] << ", " << values[1] << '\n';
		assert(values.size() ==2);


	}
	{
		std::vector<std::string> column = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "1"};
		std::vector<std::string> expected = {"6", "7", "8", "9"};
		auto compressedColumn = Huffman::compress<std::string, 64>(column);
		auto compressedPair = std::make_pair(std::get<0>(compressedColumn), std::get<1>(compressedColumn));
		{
			auto decompressed = Huffman::decompress(compressedPair);
			assert(column == decompressed);
		}
	}

	return 0;
}