#include <memory>

template<typename _Ty>
struct MyAllocator
{
	size_t allocated = 0;

	typedef _Ty value_type;
	_Ty* allocate(std::size_t n)
	{
		allocated += n;
		return std::allocator<_Ty> {} .allocate(n);
	}

	void deallocate(_Ty* mem, std::size_t n)
	{
		allocated -= n;
		std::allocator<_Ty> {} .deallocate(mem, n);
	}

	size_t allocationInByte()
	{
		return sizeof(_Ty) * allocated;
	}
};