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
#include "dictionary.cpp"

int main(int argc, char const *argv[])
{
	{
		std::vector<int> column = {1, 2, 3, 4, 5, 6, 7, 8, 9, 1};
		std::vector<int> expected = {6, 7, 8, 9};
		auto compressedColumn = Dictionary::compress<int, uint8_t>(column);
		{
			auto decompressed = Dictionary::decompress(compressedColumn);
			assert(column == decompressed);
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
			auto pred_sum = Dictionary::sum_with_predicate_op<int, uint8_t>(compressedColumn, predicate);
			std::cout << "Sum for <6: " << pred_sum << std::endl;
			assert(pred_sum == 16);
		}
		{
			std::function<bool (int)> predicate = [](int i) {
				return i > 5;
			};
			auto pred_sum = Dictionary::sum_with_predicate_op<int, uint8_t>(compressedColumn, predicate);
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
			auto found = Dictionary::search(compressedColumn, predicate);
			for(auto v : found) {
				std::cout << v << ",";
			}
			std::cout << std::endl;
			assert(expected == found);
		}
		{
			std::function<bool (int)> predicate = [](int i) {
				return i > 10;
			};
			auto found = Dictionary::search_v2(compressedColumn, predicate);
			for(auto v : found) {
				std::cout << v << ",";
			}
			std::cout << std::endl;
			std::vector<int> expected2; 
			assert(expected2 == found);
		}
	}
	{
		std::vector<std::string> column = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "1"};
		std::vector<std::string> expected = {"6", "7", "8", "9"};
		auto compressedColumn = Dictionary::compress<std::string, uint8_t>(column);
		{
			auto decompressed = Dictionary::decompress(compressedColumn);
			assert(column == decompressed);
		}
		{
			std::function<bool (std::string)> predicate = [](std::string i) {
				return i > "5";
			};
			auto found = Dictionary::search(compressedColumn, predicate);
			for(auto v : found) {
				std::cout << v << ",";
			}
			std::cout << std::endl;
			assert(expected == found);
		}
		{
			std::function<bool (std::string)> predicate = [](std::string i) {
				return i > "5";
			};
			auto found = Dictionary::search_v2(compressedColumn, predicate);
			for(auto v : found) {
				std::cout << v << ",";
			}
			std::cout << std::endl;
			assert(expected == found);
		}
	}

	return 0;
}