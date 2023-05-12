#pragma once

#include "imui_internal.h"

#define IMUI_MEMORY_NEW( ALLOCATOR, TYPE )		(TYPE*)ImUiMemoryAlloc( ALLOCATOR, sizeof( TYPE ) )
#define IMUI_MEMORY_NEW_ZERO( ALLOCATOR, TYPE )	(TYPE*)ImUiMemoryAllocZero( ALLOCATOR, sizeof( TYPE ) )

#define IMUI_MEMORY_ARRAY_NEW( ALLOCATOR, TYPE, COUNT )			(TYPE*)ImUiMemoryAlloc( ALLOCATOR, sizeof( TYPE ) * COUNT )
#define IMUI_MEMORY_ARRAY_NEW_ZERO( ALLOCATOR, TYPE, COUNT )	(TYPE*)ImUiMemoryAllocZero( ALLOCATOR, sizeof( TYPE ) * COUNT )

#define IMUI_MEMORY_CHECK_ARRAY_CAPACITY( ALLOCATOR, ARRAY, CAPACITY, COUNT )		ImUiMemoryCheckArrayCapacity( ALLOCATOR, (void**)&ARRAY, &CAPACITY, COUNT, sizeof( *ARRAY ), false )
#define IMUI_MEMORY_CHECK_ARRAY_CAPACITY_ZERO( ALLOCATOR, ARRAY, CAPACITY, COUNT )	ImUiMemoryCheckArrayCapacity( ALLOCATOR, (void**)&ARRAY, &CAPACITY, COUNT, sizeof( *ARRAY ), true )

void*	ImUiMemoryDefaultAlloc( uintsize size, void* userData );
void*	ImUiMemoryDefaultRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData );
void*	ImUiMemoryPseudoRealloc( void* oldMemory, uintsize oldSize, uintsize newSize, void* userData );
void	ImUiMemoryDefaultFree( void* memory, void* userData );

void*	ImUiMemoryAlloc( ImUiAllocator* allocator, uintsize size );
void*	ImUiMemoryAllocZero( ImUiAllocator* allocator, uintsize size );
void*	ImUiMemoryRealloc( ImUiAllocator* allocator, void* oldMemory, uintsize oldSize, uintsize newSize );
void	ImUiMemoryFree( ImUiAllocator* allocator, const void* memory );

bool	ImUiMemoryCheckArrayCapacity( ImUiAllocator* allocator, void** memory, uintsize* capacity, uintsize requiredCapacity, uintsize elementSize, bool zero );