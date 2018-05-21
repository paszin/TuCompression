#include <string>
#include <vector>
#include <set>

namespace Dictionary {

	struct DictionaryCompressedColumn
	{
		std::set<std::string> dictionary;
		std::vector<std::size_t> attributeVector;

		DictionaryCompressedColumn(
			std::set<std::string> dictionary,
			std::vector<std::size_t> attributeVector) : dictionary(dictionary), attributeVector(attributeVector) { }
	};

	DictionaryCompressedColumn compress(std::vector<std::string> column);

}