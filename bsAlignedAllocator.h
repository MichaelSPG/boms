#pragma once

#include <stdlib.h>//_aligned_malloc


/*	Stateless allocator which forwards all allocation requests to _aligned_malloc.
*/
template <typename T, size_t Alignment = 16>
class bsAlignedAllocator : public std::allocator<T>
{
public:
	typedef T value_type;
	typedef value_type* pointer;
	typedef value_type& reference;
	typedef const value_type* const_pointer;
	typedef const value_type& const_reference;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;


	template<class Other>
	struct rebind
	{
		typedef bsAlignedAllocator<Other, Alignment> other;
	};


	bsAlignedAllocator()
	{}

	template <typename U>
	bsAlignedAllocator(const bsAlignedAllocator<U, Alignment>&)
	{}

	~bsAlignedAllocator()
	{}

	bsAlignedAllocator& operator=(const bsAlignedAllocator&)
	{
		return *this;
	}



	pointer allocate(size_type count)
	{
		const size_type sizeToAllocate = count * sizeof(value_type);
		return static_cast<pointer>(_aligned_malloc(sizeToAllocate, Alignment));
	}

	pointer allocate(size_type count, const void*)
	{
		return (allocate(count));
	}

	void deallocate(pointer ptr, size_type)
	{
		_aligned_free(ptr);
	}

	void construct(pointer ptr, const T& val)
	{
		new(ptr) T(val);
	}

	void construct(pointer ptr, value_type&& val)
	{
		new(ptr) value_type(std::forward<value_type>(val));
	}

	template<class Other>
	void construct(pointer ptr, Other&& val)
	{
		new(ptr) value_type(std::forward<Other>(val));
	}

	void destroy(pointer ptr)
	{
		//Cast to void to avoid incorrect warning about unused variable.
		(void)ptr;

		ptr->~T();
	}

	size_t max_size() const
	{
		return ~size_t(0) / sizeof(T);
	}
};