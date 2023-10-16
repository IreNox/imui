#pragma once

#include "imui/imui.h"

#include "imui_types.h"

#define IMUI_MEMORY_NEW( ALLOCATOR, TYPE )		(TYPE*)ImUiMemoryAlloc( ALLOCATOR, sizeof( TYPE ) )
#define IMUI_MEMORY_NEW_ZERO( ALLOCATOR, TYPE )	(TYPE*)ImUiMemoryAllocZero( ALLOCATOR, sizeof( TYPE ) )

#define IMUI_MEMORY_ARRAY_NEW( ALLOCATOR, TYPE, COUNT )			(TYPE*)ImUiMemoryAlloc( ALLOCATOR, sizeof( TYPE ) * COUNT )
#define IMUI_MEMORY_ARRAY_NEW_ZERO( ALLOCATOR, TYPE, COUNT )	(TYPE*)ImUiMemoryAllocZero( ALLOCATOR, sizeof( TYPE ) * COUNT )

#define IMUI_MEMORY_ARRAY_CHECK_CAPACITY( ALLOCATOR, ARRAY, CAPACITY, COUNT )		ImUiMemoryArrayCheckCapacity( ALLOCATOR, (void**)&ARRAY, &CAPACITY, COUNT, sizeof( *ARRAY ), false )
#define IMUI_MEMORY_ARRAY_CHECK_CAPACITY_ZERO( ALLOCATOR, ARRAY, CAPACITY, COUNT )	ImUiMemoryArrayCheckCapacity( ALLOCATOR, (void**)&ARRAY, &CAPACITY, COUNT, sizeof( *ARRAY ), true )

#define IMUI_MEMORY_ARRAY_REMOVE_UNSORTED( ARRAY, COUNT, INDEX )		ImUiMemoryArrayRemoveElementUnsorted( ARRAY, &COUNT, INDEX, sizeof( *ARRAY ), false )
#define IMUI_MEMORY_ARRAY_REMOVE_UNSORTED_ZERO( ARRAY, COUNT, INDEX )	ImUiMemoryArrayRemoveElementUnsorted( ARRAY, &COUNT, INDEX, sizeof( *ARRAY ), true )

#define IMUI_MEMORY_ARRAY_FREE( ALLOCATOR, ARRAY, CAPACITY )			ImUiMemoryArrayFree( ALLOCATOR, (void**)&ARRAY, &CAPACITY )

void	ImUiMemoryAllocatorPrepare( ImUiAllocator* targetAllocator, const ImUiAllocator* sourceAllocator );
void	ImUiMemoryAllocatorFinalize( ImUiAllocator* targetAllocator, const ImUiAllocator* sourceAllocator );

void*	ImUiMemoryDefaultAlloc( uintsize size, void* userData );
void*	ImUiMemoryDefaultRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData );
void*	ImUiMemoryPseudoRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData );
void	ImUiMemoryDefaultFree( void* memory, void* userData );

void*	ImUiMemoryAlloc( ImUiAllocator* allocator, uintsize size );
void*	ImUiMemoryAllocZero( ImUiAllocator* allocator, uintsize size );
void*	ImUiMemoryRealloc( ImUiAllocator* allocator, void* oldMemory, uintsize oldSize, uintsize newSize );
void	ImUiMemoryFree( ImUiAllocator* allocator, const void* memory );

bool	ImUiMemoryArrayCheckCapacity( ImUiAllocator* allocator, void** memory, uintsize* capacity, uintsize requiredCapacity, uintsize elementSize, bool zero );
void	ImUiMemoryArrayRemoveElementUnsorted( void* memory, uintsize* arrayCount, uintsize elementIndex, uintsize elementSize, bool zero );
void	ImUiMemoryArrayFree( ImUiAllocator* allocator, void** memory, uintsize* capacity );
