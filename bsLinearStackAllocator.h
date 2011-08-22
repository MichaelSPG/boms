#pragma once

#include <stdlib.h>//malloc/free


/*	Linear stack allocator, with size determined at compile-time.
	This allocator stores all of its memory in a single array which is allocated once.

	When calling allocate(n), a single pointer increment is all that is required to
	return the requested memory, making individual allocations very fast.

	Be aware that making instances of this class with big memory sizes can easily create
	stack overflows, so only use it when you know that will not be an issue.

	Memory allocated from this allocator does not need to be freed, as the whole buffer
	will be popped off the stack when the allocator goes out of scope.
*/
template <size_t memorySize>
class bsLinearStackAllocator
{
	static_assert(memorySize != 0, "Cannot create an allocator of size 0");

public:
	bsLinearStackAllocator()
		: mBufferHead(mBuffer)
	{
	}

	~bsLinearStackAllocator()
	{
	}

	/*	Allocates enough memory for the requested amount of objects (sizeof(T) * count).
		Returns null if there is not enough free memory.

		allocate<int>(3); allocates 3 ints.
	*/
	template <typename T>
	inline T* allocate(size_t count)
	{
		return static_cast<T*>(internalAllocate(sizeof(T) * count));
	}

	/*	Allocates a certain amount of bytes.
		Returns null if there is not enough memory.

		allocate(63) allocates 63 bytes.
	*/
	inline char* allocate(size_t bytes)
	{
		return static_cast<char*>(internalAllocate(bytes));
	}

	/*	Resets allocator. All previously allocated memory is invalid after calling this.
	*/
	inline void clear()
	{
		mBufferHead = mBuffer;
	}

	/*	Returns true if the allocator has enough space left to allocate this many bytes.
	*/
	inline bool canFit(size_t bytes) const
	{
		return mBufferHead + bytes <= mBuffer + memorySize;
	}
	

private:
	//Non-copyable.
	bsLinearStackAllocator(const bsLinearStackAllocator&);
	bsLinearStackAllocator& operator=(const bsLinearStackAllocator&);

	/*	Checks if there is enough free memory to allocate the requested bytes, and
		allocates that much memory if so.
		Returns null if allocation failed.
	*/
	void* internalAllocate(size_t bytes)
	{
		if (!canFit(bytes))
		{
			return nullptr;
		}

		char* ret = mBufferHead;
		//Move current pointer to after this newly allocated block of memory.
		mBufferHead += bytes;
		return ret;
	}

	//Beginning of memory block.
	char mBuffer[memorySize];
	//Current head of memory block.
	char* mBufferHead;
};
