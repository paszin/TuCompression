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
#include "allocator.cpp"
#include "benchmark.cpp"
#include "dictionary.cpp"

int main(int argc, char const *argv[])
{
	{
		using namespace std::placeholders;
		std::vector<int> column = {1, 2, 3, 4, 5, 6, 7, 8, 9, 1};
		// where_copy_op
		std::function<bool (int)> predicate = [](int i) {
			return i > 5;
		};
		auto func = [predicate](std::pair<std::vector<int>, std::vector<uint8_t>> &col) -> std::vector<int> {
			return Dictionary::where_copy_op(col, predicate);
		};
		auto compressedColumn = Dictionary::compress<int, uint8_t>(column);
		auto runtimes = Dictionary::benchmark_op_with_dtype<int, uint8_t, std::vector<int>>(compressedColumn, 1, 1, false, func);
		for (auto r : runtimes) {
			std::cout << r << std::endl;
		}
	}
	std::cout << "#### TEST WITH INT ####" << std::endl;
	{
		std::vector<int> column = {1, 2, 3, 4, 5, 6, 7, 8, 9, 1};
		std::vector<int> expected = {6, 7, 8, 9};
		auto compressedColumn = Dictionary::compress<int, uint8_t>(column);
		{
			auto decompressed = Dictionary::decompress(compressedColumn);
			assert(column == decompressed);
		}
		{
			std::vector<size_t> indices = {0, 3, 7};
			std::vector<int> expectedPartial = {1, 4, 8};
			auto partiallyDecompressed = Dictionary::partial_decompress(compressedColumn, indices);
			for (auto v : partiallyDecompressed) {
				std::cout << v << ",";
			}
			std::cout << std::endl;
			assert(partiallyDecompressed == expectedPartial);
		}
		{
			auto sum = Dictionary::sum_op(compressedColumn);
			std::cout << "Sum: " << sum << std::endl;
			assert(sum == 46);
		}
		{
			std::function<bool (int)> predicate = [](int i) {
				return i < 6;
			};
			auto pred_sum = Dictionary::sum_where_copy_op<int, uint8_t>(compressedColumn, predicate);
			std::cout << "Sum for <6: " << pred_sum << std::endl;
			assert(pred_sum == 16);
		}
		{
			std::function<bool (int)> predicate = [](int i) {
				return i > 5;
			};
			auto pred_sum = Dictionary::sum_where_copy_op<int, uint8_t>(compressedColumn, predicate);
			std::cout << "Sum for >5: " << pred_sum << std::endl;
			assert(pred_sum == 30);
		}
		{
			auto avg = Dictionary::avg_op(compressedColumn);
			std::cout << "Avg: " << avg << std::endl;
			assert(avg == 4.6f);
		}
		{
			std::function<bool (int)> predicate = [](int i) {
				return i > 5;
			};
			auto found = Dictionary::where_copy_op(compressedColumn, predicate);
			for (auto v : found) {
				std::cout << v << ",";
			}
			std::cout << std::endl;
			assert(expected == found);
		}
		{
			std::function<bool (int)> predicate = [](int i) {
				return i > 10;
			};
			auto found = Dictionary::where_view_op(compressedColumn, predicate);
			for (auto v : found) {
				std::cout << v << ",";
			}
			std::cout << std::endl;
			std::vector<int> expected2;
			assert(expected2 == found);
		}
	}
	std::cout << "#### TEST WITH STD::STRING ####" << std::endl;
	{
		std::vector<std::string> column = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "1"};
		std::vector<std::string> expected = {"6", "7", "8", "9"};
		auto compressedColumn = Dictionary::compress<std::string, uint8_t>(column);
		{
			auto decompressed = Dictionary::decompress(compressedColumn);
			assert(column == decompressed);
		}
		{
			std::vector<size_t> indices = {0, 3, 7};
			std::vector<std::string> expectedPartial = {"1", "4", "8"};
			auto partiallyDecompressed = Dictionary::partial_decompress(compressedColumn, indices);
			for (auto v : partiallyDecompressed) {
				std::cout << v << ",";
			}
			std::cout << std::endl;
			assert(partiallyDecompressed == expectedPartial);
		}
		{
			std::function<bool (std::string)> predicate = [](std::string i) {
				return i > "5";
			};
			auto found = Dictionary::where_copy_op(compressedColumn, predicate);
			for (auto v : found) {
				std::cout << v << ",";
			}
			std::cout << std::endl;
			assert(expected == found);
		}
		{
			std::function<bool (std::string)> predicate = [](std::string i) {
				return i > "5";
			};
			auto found = Dictionary::where_view_op(compressedColumn, predicate);
			for (auto v : found) {
				std::cout << v << ",";
			}
			std::cout << std::endl;
			assert(expected == found);
		}
	}
	std::cout << "#### TEST WITH STD::STRING + DATES ####" << std::endl;
	{
		std::vector<std::string> column = {"1996-01-02", "1995-01-03", "1995-01-04", "1995-01-05"};
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
		auto compressedColumn = Dictionary::compress<std::time_t, uint8_t>(convertedColumn);
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
		auto where = Dictionary::where_view_op(compressedColumn, predicate);
		for (auto t : convertedColumn) {
			std::cout << t << std::endl;
		}
		std::cout << where.size() << std::endl;
		// assert(where.size() == 3);
	}
	return 0;
}