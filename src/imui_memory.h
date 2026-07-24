#pragma once

#include "imui/imui.h"

#include "imui_types.h"

#define IMUI_MEMORY_NEW( ALLOCATOR, TYPE )		(TYPE*)imuiMemoryAlloc( ALLOCATOR, sizeof( TYPE ) )
#define IMUI_MEMORY_NEW_ZERO( ALLOCATOR, TYPE )	(TYPE*)imuiMemoryAllocZero( ALLOCATOR, sizeof( TYPE ) )

#define IMUI_MEMORY_ARRAY_NEW( ALLOCATOR, TYPE, COUNT )			(TYPE*)imuiMemoryAlloc( ALLOCATOR, sizeof( TYPE ) * COUNT )
#define IMUI_MEMORY_ARRAY_NEW_ZERO( ALLOCATOR, TYPE, COUNT )	(TYPE*)imuiMemoryAllocZero( ALLOCATOR, sizeof( TYPE ) * COUNT )

#define IMUI_MEMORY_ARRAY_CHECK_CAPACITY( ALLOCATOR, ARRAY, CAPACITY, COUNT )		imuiMemoryArrayCheckCapacity( ALLOCATOR, (void**)&ARRAY, &CAPACITY, COUNT, sizeof( *ARRAY ), false )
#define IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( ALLOCATOR, ARRAY, CAPACITY, COUNT )	imuiMemoryArrayCheckCapacity( ALLOCATOR, (void**)&ARRAY, &CAPACITY, COUNT, sizeof( *ARRAY ), true )

#define IMUI_MEMORY_ARRAY_REMOVE_UNSORTED( ARRAY, COUNT, INDEX )		imuiMemoryArrayRemoveElementUnsorted( ARRAY, &COUNT, INDEX, sizeof( *ARRAY ), false )
#define IMUI_MEMORY_ARRAY_REMOVE_UNSORTED_ZERO( ARRAY, COUNT, INDEX )	imuiMemoryArrayRemoveElementUnsorted( ARRAY, &COUNT, INDEX, sizeof( *ARRAY ), true )

#define IMUI_MEMORY_ARRAY_SHRINK( ALLOCATOR, ARRAY, CAPACITY, COUNT )	imuiMemoryArrayShrink( ALLOCATOR, (void**)&ARRAY, &CAPACITY, COUNT, sizeof( *ARRAY ) )

#define IMUI_MEMORY_ARRAY_FREE( ALLOCATOR, ARRAY, CAPACITY )			imuiMemoryArrayFree( ALLOCATOR, (void**)&ARRAY, &CAPACITY )

void	imuiMemoryAllocatorPrepare( ImuiAllocator* targetAllocator, const ImuiAllocator* sourceAllocator );
void	imuiMemoryAllocatorFinalize( ImuiAllocator* targetAllocator, const ImuiAllocator* sourceAllocator );

void*	imuiMemoryDefaultAlloc( uintsize size, void* userData );
void*	imuiMemoryDefaultRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData );
void*	imuiMemoryPseudoRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData );
void	imuiMemoryDefaultFree( void* memory, void* userData );

void*	imuiMemoryAlloc( ImuiAllocator* allocator, uintsize size );
void*	imuiMemoryAllocZero( ImuiAllocator* allocator, uintsize size );
void*	imuiMemoryRealloc( ImuiAllocator* allocator, void* oldMemory, uintsize oldSize, uintsize newSize );
void	imuiMemoryFree( ImuiAllocator* allocator, const void* memory );

bool	imuiMemoryArrayCheckCapacity( ImuiAllocator* allocator, void** memory, uintsize* capacity, uintsize requiredCapacity, uintsize elementSize, bool zero );
void	imuiMemoryArrayRemoveElementUnsorted( void* memory, uintsize* arrayCount, uintsize elementIndex, uintsize elementSize, bool zero );
void	imuiMemoryArrayShrink( ImuiAllocator* allocator, void** memory, uintsize* capacity, uintsize count, uintsize elementSize );
void	imuiMemoryArrayFree( ImuiAllocator* allocator, void** memory, uintsize* capacity );
