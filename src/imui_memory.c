#include "imui_memory.h"

#include <stdlib.h>
#include <string.h>

void imuiMemoryAllocatorPrepare( ImuiAllocator* targetAllocator, const ImuiAllocator* sourceAllocator )
{
	*targetAllocator = *sourceAllocator;

	if( targetAllocator->mallocFunc == NULL ||
		targetAllocator->freeFunc == NULL )
	{
		targetAllocator->mallocFunc		= imuiMemoryDefaultAlloc;
		targetAllocator->reallocFunc	= imuiMemoryDefaultRealloc;
		targetAllocator->freeFunc		= imuiMemoryDefaultFree;
		targetAllocator->userData		= NULL;
		targetAllocator->internalData	= NULL;
	}
	else
	{
		targetAllocator->internalData	= sourceAllocator->userData;
	}
}

void imuiMemoryAllocatorFinalize( ImuiAllocator* targetAllocator, const ImuiAllocator* sourceAllocator )
{
	*targetAllocator = *sourceAllocator;

	if( targetAllocator->reallocFunc == NULL )
	{
		targetAllocator->reallocFunc	= imuiMemoryPseudoRealloc;
		targetAllocator->internalData	= targetAllocator;
	}
}

void* imuiMemoryDefaultAlloc( uintsize size, void* userData )
{
	(void)userData;

	return malloc( size );
}

void* imuiMemoryDefaultRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData )
{
	(void)oldSize;
	(void)userData;

	return realloc( oldMemory, newSize );
}

void* imuiMemoryPseudoRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData )
{
	(void)userData;

	ImuiAllocator* allocator = (ImuiAllocator*)userData;
	void* newMemory = imuiMemoryAlloc( allocator, newSize );
	if( newMemory == NULL )
	{
		imuiMemoryFree( allocator, oldMemory );
		return NULL;
	}

	memcpy( newMemory, oldMemory, oldSize < newSize ? oldSize : newSize );
	imuiMemoryFree( allocator, oldMemory );

	return newMemory;
}

void imuiMemoryDefaultFree( void* memory, void* userData )
{
	(void)userData;

	free( memory );
}

void* imuiMemoryAlloc( ImuiAllocator* allocator, uintsize size )
{
	return allocator->mallocFunc( size, allocator->userData );
}

void* imuiMemoryAllocZero( ImuiAllocator* allocator, uintsize size )
{
	void* memory = imuiMemoryAlloc( allocator, size );
	if( memory == NULL )
	{
		return NULL;
	}

	memset( memory, 0, size );
	return memory;
}

void* imuiMemoryRealloc( ImuiAllocator* allocator, void* oldMemory, uintsize oldSize, uintsize newSize )
{
	return allocator->reallocFunc( oldMemory, oldSize, newSize, allocator->internalData );
}

void imuiMemoryFree( ImuiAllocator* allocator, const void* memory )
{
	allocator->freeFunc( (void*)memory, allocator->userData );
}

bool imuiMemoryArrayCheckCapacity( ImuiAllocator* allocator, void** memory, uintsize* capacity, uintsize requiredCapacity, uintsize elementSize, bool zero )
{
	if( *capacity >= requiredCapacity )
	{
		return true;
	}

	uintsize nextCapacity = IMUI_NEXT_POWER_OF_TWO( requiredCapacity );
	if( nextCapacity < IMUI_DEFAULT_ARRAY_CAPACITY )
	{
		nextCapacity = IMUI_DEFAULT_ARRAY_CAPACITY;
	}

	const uintsize oldSize = *capacity * elementSize;
	const uintsize newSize = nextCapacity * elementSize;
	void* newMemory = imuiMemoryRealloc( allocator, *memory, oldSize, newSize );
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

void imuiMemoryArrayRemoveElementUnsorted( void* memory, uintsize* arrayCount, uintsize elementIndex, uintsize elementSize, bool zero )
{
	IMUI_ASSERT( elementIndex < *arrayCount );

	uint8* bytes = (uint8*)memory;
	uint8* element = bytes + (elementIndex * elementSize);

	if( *arrayCount > 1 &&
		elementIndex != *arrayCount - 1 )
	{
		uint8* lastElement = bytes + ((*arrayCount - 1) * elementSize);
		memcpy( element, lastElement, elementSize );

		if( zero )
		{
			memset( lastElement, 0, elementSize );
		}
	}
	else if( zero )
	{
		memset( element, 0, elementSize );
	}

	(*arrayCount)--;
}

void imuiMemoryArrayShrink( ImuiAllocator* allocator, void** memory, uintsize* capacity, uintsize count, uintsize elementSize )
{
	uintsize shrinkCapacity = *capacity >> 1;
	if( shrinkCapacity > 0 &&
		shrinkCapacity < IMUI_DEFAULT_ARRAY_CAPACITY )
	{
		shrinkCapacity = IMUI_DEFAULT_ARRAY_CAPACITY;
	}

	if( count >= shrinkCapacity ||
		shrinkCapacity >= *capacity )
	{
		return;
	}

	*memory = imuiMemoryRealloc( allocator, *memory, *capacity * elementSize, shrinkCapacity * elementSize );

	if( !*memory )
	{
		return;
	}

	*capacity = shrinkCapacity;
}

void imuiMemoryArrayFree( ImuiAllocator* allocator, void** memory, uintsize* capacity )
{
	imuiMemoryFree( allocator, *memory );
	*memory		= NULL;
	*capacity	= 0u;
}
