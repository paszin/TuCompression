#include <memory>

template <typename T>
class MyAllocator
{
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	MyAllocator() {}
	~MyAllocator() {}

	//template <class U> struct rebind { typedef MyAllocator<U> other; };
	template <class U> MyAllocator(const MyAllocator<U>& inner) {
		this->allocated = inner.allocated;
	}

	pointer address(reference x) const {return &x;}
	const_pointer address(const_reference x) const {return &x;}
	size_type max_size() const throw() {return size_t(-1) / sizeof(value_type);}

	size_t allocated = 0;

	pointer allocate(size_type n)
	{
		this->allocated += n * sizeof(T);
		return static_cast<pointer>(malloc(n * sizeof(T)));
	}

	void deallocate(pointer p, size_type n)
	{
		this->allocated -= n * sizeof(T);
		free(p);
	}

	void construct(pointer p, const T& val)
	{
		new(static_cast<void*>(p)) T(val);
	}

	void construct(pointer p)
	{
		new(static_cast<void*>(p)) T();
	}

	void destroy(pointer p)
	{
		p->~T();
	}

	size_t allocationInByte()
	{
		return this->allocated;
	}
};

size_t sizeOfString(std::string str) {
	std::basic_string<char, std::char_traits<char>, MyAllocator<char>> strWithAlloc(str);
	return strWithAlloc.get_allocator().allocationInByte() + sizeof(str);
}