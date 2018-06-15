#include <iostream>
#include <vector>
#include <memory>

template<typename _Ty>
struct MyAllocator
{
	size_t allocated = 0;

	typedef _Ty value_type;
	_Ty* allocate(std::size_t n)
	{
		std::cout << "Allocating: " << n << std::endl;
		allocated += n;
		return std::allocator<_Ty> {} .allocate(n);
	}

	void deallocate(_Ty* mem, std::size_t n)
	{
		std::cout << "Deallocating: " << n << std::endl;
		allocated -= n;
		std::allocator<_Ty> {} .deallocate(mem, n);
	}

	size_t allocationInByte()
	{
		return sizeof(_Ty) * allocated;
	}
};

int main(int argc, char const *argv[])
{
	std::vector<int, MyAllocator<int>> vec(MyAllocator<int> {});
	vec.push_back(1);
	vec.push_back(1);
	vec.push_back(1);
	vec.push_back(1);
	vec.push_back(1);
	vec.push_back(1);
	std::cout << "Done" << std::endl;
	std::cout << "Allocated " << vec.get_allocator().allocationInByte() << " Byte" << std::endl;
	std::cout << "Sizeof int: " << sizeof(int) << std::endl;

	std::vector<int, MyAllocator<int>> vec2(vec.begin(), vec.end(), MyAllocator<int> {});
	std::cout << "Allocated " << vec2.get_allocator().allocationInByte() << " Byte" << std::endl;

	std::vector<int, MyAllocator<int>> vec3(vec, MyAllocator<int> {});
	std::cout << "Allocated " << vec3.get_allocator().allocationInByte() << " Byte" << std::endl;

	return 0;
}