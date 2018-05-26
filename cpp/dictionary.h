#pragma once

#include <string>
#include <vector>
#include "compressed.h"

namespace Dictionary {

	CompressedColumn compress(const std::vector<std::string> &column);

	std::vector<std::string> decompress(const CompressedColumn &column);

	size_t size(const CompressedColumn &column);

}