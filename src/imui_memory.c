#include "imui_memory.h"

#include <stdlib.h>
#include <string.h>

void ImUiMemoryAllocatorPrepare( ImUiAllocator* targetAllocator, const ImUiAllocator* sourceAllocator )
{
	*targetAllocator = *sourceAllocator;

	if( targetAllocator->mallocFunc == NULL ||
		targetAllocator->freeFunc == NULL )
	{
		targetAllocator->mallocFunc		= ImUiMemoryDefaultAlloc;
		targetAllocator->reallocFunc	= ImUiMemoryDefaultRealloc;
		targetAllocator->freeFunc		= ImUiMemoryDefaultFree;
		targetAllocator->userData		= NULL;
		targetAllocator->internalData	= NULL;
	}
	else
	{
		targetAllocator->internalData	= sourceAllocator->userData;
	}
}

void ImUiMemoryAllocatorFinalize( ImUiAllocator* targetAllocator, const ImUiAllocator* sourceAllocator )
{
	*targetAllocator = *sourceAllocator;

	if( targetAllocator->reallocFunc == NULL )
	{
		targetAllocator->reallocFunc	= ImUiMemoryPseudoRealloc;
		targetAllocator->internalData	= targetAllocator;
	}
}

void* ImUiMemoryDefaultAlloc( uintsize size, void* userData )
{
	(void)userData;

	return malloc( size );
}

void* ImUiMemoryDefaultRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData )
{
	(void)oldSize;
	(void)userData;

	return realloc( oldMemory, newSize );
}

void* ImUiMemoryPseudoRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData )
{
	(void)userData;

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
	(void)userData;

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

bool ImUiMemoryArrayCheckCapacity( ImUiAllocator* allocator, void** memory, uintsize* capacity, uintsize requiredCapacity, uintsize elementSize, bool zero )
{
	if( *capacity >= requiredCapacity )
	{
		return true;
	}

	uintsize nextCapacity = *capacity;
	if( nextCapacity < IMUI_DEFAULT_ARRAY_CAPACITY )
	{
		nextCapacity = IMUI_DEFAULT_ARRAY_CAPACITY;
	}
	while( nextCapacity < requiredCapacity )
	{
		nextCapacity <<= 1u;
	}

	const uintsize oldSize = *capacity * elementSize;
	const uintsize newSize = nextCapacity * elementSize;
	void* newMemory = ImUiMemoryRealloc( allocator, *memory, oldSize, newSize );
	if( !newMemory )
	{
		return false;
	}

	if( zero )
	{
		uint8* data = (uint8*)newMemory;
		memset( data + oldSize, 0, newSize - oldSize );
	}

	*memory = newMemory;
	*capacity = nextCapacity;
	return true;
}

void ImUiMemoryArrayRemoveElementUnsorted( void* memory, uintsize* arrayCount, uintsize elementIndex, uintsize elementSize, bool zero )
{
	IMUI_ASSERT( elementIndex < *arrayCount );

	uint8* bytes = (uint8*)memory;
	uint8* element = bytes + (elementIndex * elementSize);

	if( *arrayCount > 1u &&
		elementIndex != *arrayCount - 1u )
	{
		uint8* lastElement = bytes + ((*arrayCount - 1u) * elementSize);
		memcpy( element, lastElement, elementSize );

		if( zero )
		{
			memset( lastElement, 0, elementSize );
		}
	}
	else if( zero &&
		*arrayCount == 1u )
	{
		memset( element, 0, elementSize );
	}

	(*arrayCount)--;
}

void ImUiMemoryArrayShrink( ImUiAllocator* allocator, void** memory, uintsize* capacity, uintsize count, uintsize elementSize )
{
	const uintsize shrinkCapacity = *capacity >> 1;
	if( count >= shrinkCapacity )
	{
		return;
	}

	*memory = ImUiMemoryRealloc( allocator, *memory, *capacity * elementSize, shrinkCapacity * elementSize );

	if( !*memory )
	{
		*capacity = 0;
	}
	else
	{
		*capacity = shrinkCapacity;
	}
}

void ImUiMemoryArrayFree( ImUiAllocator* allocator, void** memory, uintsize* capacity )
{
	ImUiMemoryFree( allocator, *memory );
	*memory		= NULL;
	*capacity	= 0u;
}
