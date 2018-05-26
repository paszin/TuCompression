#pragma once

#include <string>
#include <vector>
#include "compressed.h"
#include "encoder.h"

template <typename D, typename A>
class DictionaryEncoder: public Encoder<D, A>
{
public:
	virtual CompressedColumn<D, A> compress(const std::vector<D> &column) override;

	virtual std::vector<D> decompress(const CompressedColumn<D, A> &column) override;

	virtual size_t size(const CompressedColumn<D, A> &column) override;
};

#include "dictionary.hpp"