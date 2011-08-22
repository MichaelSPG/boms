#pragma once

#include <stdlib.h>//malloc/free


/*	Linear heap allocator.
	This allocator stores its memory buffer on the heap. This buffer is a single
	contiguous block of memory, which is allocated when an instance of this class is
	constructed.

	When calling allocate(n), a single pointer increment is all that is required to
	return the requested memory, making individual allocations very fast.
	This allocator can also prevent fragmentation, for instance if many small allocations
	with a short lifetime are needed.

	Memory allocated from this allocator does not need to be freed manually, as the whole
	buffer will be freed by the destructor (unless takeOwnershipOfAllocatedMemory is called).
*/
class bsLinearHeapAllocator
{
public:
	inline explicit bsLinearHeapAllocator(size_t memorySize)
		: mBuffer(static_cast<char*>(malloc(memorySize)))
		, mBufferHead(mBuffer)
		, mMemorySize(memorySize)
	{}

	inline ~bsLinearHeapAllocator()
	{
		free(mBuffer);
	}

	/*	Allocates enough memory for the requested amount of objects (sizeof(T) * count).
		Returns null if there is not enough free memory.
	*/
	template <typename T>
	inline T* allocate(size_t count)
	{
		return static_cast<T*>(internalAllocate(sizeof(T) * count));
	}

	/*	Allocates bytes bytes.
		Returns null if there is not enough memory.
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
		return mBufferHead + bytes <= mBuffer + mMemorySize;
	}

	/*	Takes ownership of this allocator's data.
		By default, the allocator will free all allocated memory in its destructor.
		Calling this function overrides that behavior, allowing you to keep the data in
		memory after this allocator has expired.
		
		The allocator is invalid after calling this function.
		
		Returns a pointer to the data buffer. Don't forget to free this buffer.
	*/
	inline void* takeOwnershipOfAllocatedMemory()
	{
		void* buffer = mBuffer;
		//Null out own data so destructor's free won't it.
		mBuffer = nullptr;
		mBufferHead = nullptr;
		
		return buffer;
	}

	/*	Returns how many remaining free bytes this allocator contains.
	*/
	inline size_t getNumRemainingBytes() const
	{
		return static_cast<size_t>((mBuffer + mMemorySize) - mBufferHead);
	}
	

private:
	//Non-copyable.
	bsLinearHeapAllocator(const bsLinearHeapAllocator&);
	bsLinearHeapAllocator& operator=(const bsLinearHeapAllocator&);

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

	//Beginning of actual memory block.
	char* mBuffer;
	//Current head of memory block.
	char* mBufferHead;
	//Size of the buffer.
	size_t mMemorySize;
};
