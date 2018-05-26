#pragma once

#include <string>
#include <vector>
#include "compressed.h"

namespace Huffman {

	CompressedColumn compress(std::vector<std::string> column);

	std::vector<std::string> decompress(CompressedColumn column);

}