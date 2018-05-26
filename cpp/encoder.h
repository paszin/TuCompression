#include "compressed.h"


template <typename D, typename A>
class Encoder
{
public:
	virtual CompressedColumn<D, A> compress(const std::vector<D> &column);

	virtual std::vector<D> decompress(const CompressedColumn<D, A> &column);

	virtual size_t size(const CompressedColumn<D, A> &column);
};