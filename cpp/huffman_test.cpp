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
	//std::bitset<64> bs(pow(2, 60));
	//std::cout << "bs: " << bs << std::endl;
	//std::cout << "length: " << Huffman::getCodeLength(bs) << std::endl;
	std::bitset<8> b(42);
	std::cout << b.to_string() << '\n';
	{
		std::vector<int> column = {1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 5, 6};

		for(size_t i = 0; i < 40; i++)
		{
			column.push_back(i);
		}

		std::vector<int> expected = {6, 7, 8, 9};
		auto compressedColumn = Huffman::compress<int, 64>(column);
		//std::cout << "compressed: " << compressedColumn << std::endl;
		{
			auto decompressed = Huffman::decompress2(compressedColumn);
			//assert(column == decompressed);
		}
	}
	{
		std::vector<std::string> column = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "1"};
		std::vector<std::string> expected = {"6", "7", "8", "9"};
		//auto compressedColumn = Huffman::compress<std::string, 64>(column);
		{
			//auto decompressed = Huffman::decompress(compressedColumn);
			//assert(column == decompressed);
		}
	}

	return 0;
}