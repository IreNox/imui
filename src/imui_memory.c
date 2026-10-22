#include "imui_memory.h"

#include <stdlib.h>
#include <string.h>

static void*	imuiMemoryDefaultAlloc( uintsize size, void* userData );
static void*	imuiMemoryDefaultRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData );
static void*	imuiMemoryPseudoRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData );
static void		imuiMemoryDefaultFree( void* memory, void* userData );

void imuiMemoryAllocatorPrepare( ImuiAllocator* targetAllocator, const ImuiAllocatorParameters* allocatorParameters )
{
	targetAllocator->mallocFunc		= allocatorParameters->mallocFunc;
	targetAllocator->reallocFunc	= allocatorParameters->reallocFunc;
	targetAllocator->freeFunc		= allocatorParameters->freeFunc;
	targetAllocator->userData		= allocatorParameters->userData;

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
		targetAllocator->internalData	= allocatorParameters->userData;
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

void* imuiMemoryAlloc( ImuiAllocator* allocator, uintsize size )
{
#if IMUI_DEBUG
	allocator->allocationCount++;
	allocator->allocationSize += size;
	allocator->maxAllocationCount = IMUI_MAX( allocator->maxAllocationCount, allocator->allocationCount );
	allocator->maxAllocationSize = IMUI_MAX( allocator->maxAllocationSize, allocator->allocationSize );

	uintsize* mem = (uintsize*)allocator->mallocFunc( size + sizeof( uintsize ), allocator->userData );
	if( !mem )
	{
		return NULL;
	}

	*mem = size;

	return mem + 1u;
#else
	return allocator->mallocFunc( size, allocator->userData );
#endif
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
#if IMUI_DEBUG
	allocator->allocationSize -= oldSize;
	allocator->allocationSize += newSize;
	allocator->maxAllocationSize = IMUI_MAX( allocator->maxAllocationSize, allocator->allocationSize );

	uintsize* oldSizeMem = (uintsize*)oldMemory;
	if( oldSizeMem )
	{
		oldSizeMem--;
	}

	uintsize* mem = (uintsize*)allocator->reallocFunc( oldSizeMem, oldSize + sizeof( uintsize ), newSize + sizeof(uintsize), allocator->internalData );
	if( !mem )
	{
		return NULL;
	}

	*mem = newSize;

	return mem + 1u;
#else
	return allocator->reallocFunc( oldMemory, oldSize, newSize, allocator->internalData );
#endif
}

void imuiMemoryFree( ImuiAllocator* allocator, const void* memory )
{
#if IMUI_DEBUG
	if( !memory )
	{
		return;
	}

	uintsize* size = (uintsize*)memory;
	size--;

	allocator->allocationCount--;
	allocator->allocationSize -= *size;

	memory = size;
#endif

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

static void* imuiMemoryDefaultAlloc( uintsize size, void* userData )
{
	return malloc( size );
}

static void* imuiMemoryDefaultRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData )
{
	return realloc( oldMemory, newSize );
}

static void* imuiMemoryPseudoRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData )
{
	ImuiAllocator* allocator = (ImuiAllocator*)userData;
	void* newMemory = imuiMemoryAlloc( allocator, newSize );
	if( newMemory == NULL )
	{
		imuiMemoryFree( allocator, oldMemory );
		return NULL;
	}

	const uintsize copySize = IMUI_MIN( oldSize, newSize );
	memcpy( newMemory, oldMemory, copySize );
	imuiMemoryFree( allocator, oldMemory );

	return newMemory;
}

static void imuiMemoryDefaultFree( void* memory, void* userData )
{
	free( memory );
}
