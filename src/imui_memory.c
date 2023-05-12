#include "imui_memory.h"

#include <stdlib.h>
#include <string.h>

void* ImUiMemoryDefaultAlloc( uintsize size, void* userData )
{
	return malloc( size );
}

void* ImUiMemoryDefaultRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData )
{
	return realloc( oldMemory, newSize );
}

void* ImUiMemoryPseudoRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData )
{
	ImUiAllocator* allocator = (ImUiAllocator*)userData;
	void* newMemory = ImUiMemoryAlloc( allocator, newSize );
	if( newMemory == NULL )
	{
		ImUiMemoryFree( allocator, oldMemory );
		return NULL;
	}

	memcpy( newMemory, oldMemory, oldSize );
	ImUiMemoryFree( allocator, oldMemory );

	return newMemory;
}

void ImUiMemoryDefaultFree( void* memory, void* userData )
{
	free( memory );
}

void* ImUiMemoryAlloc( ImUiAllocator* allocator, uintsize size )
{
	return allocator->mallocFunc( size, allocator->userData );
}

void* ImUiMemoryAllocZero( ImUiAllocator* allocator, uintsize size )
{
	void* memory = ImUiMemoryAlloc( allocator, size );
	if( memory == NULL )
	{
		return NULL;
	}

	memset( memory, 0, size );
	return memory;
}

void* ImUiMemoryRealloc( ImUiAllocator* allocator, void* oldMemory, uintsize oldSize, uintsize newSize )
{
	return allocator->reallocFunc( oldMemory, oldSize, newSize, allocator->internalData );
}

void ImUiMemoryFree( ImUiAllocator* allocator, const void* memory )
{
	allocator->freeFunc( (void*)memory, allocator->userData );
}

bool ImUiMemoryCheckArrayCapacity( ImUiAllocator* allocator, void** memory, uintsize* capacity, uintsize requiredCapacity, uintsize elementSize, bool zero )
{
	if( *capacity > requiredCapacity )
	{
		return true;
	}

	uintsize nextCapacity = *capacity;
	while( nextCapacity < requiredCapacity )
	{
		nextCapacity <<= 1u;
		if( nextCapacity < IMUI_DEFAULT_ARRAY_CAPACITY )
		{
			nextCapacity = IMUI_DEFAULT_ARRAY_CAPACITY;
		}
	}

	const uintsize oldSize = *capacity * elementSize;
	const uintsize newSize = nextCapacity * elementSize;
	*memory = ImUiMemoryRealloc( allocator, *memory, oldSize, newSize );

	if( *memory )
	{
		if( zero )
		{
			uint8* data = (uint8*)*memory;
			memset( data + oldSize, 0, newSize - oldSize );
		}

		*capacity = nextCapacity;
	}

	return *memory != NULL;
}
