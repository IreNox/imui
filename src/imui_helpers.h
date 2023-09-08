#pragma once

#include "imui/imui.h"
#include "imui_types.h"

typedef struct ImUiAllocator ImUiAllocator;
typedef struct ImUiStringPoolChunk ImUiStringPoolChunk;
typedef struct ImUiStringPoolKey ImUiStringPoolKey;

typedef struct ImUiHashTable ImUiHashTable;
struct ImUiHashTable
{
	ImUiAllocator*			allocator;
	uint32*					indices;
	uint8*					keys;
	uintsize				keyCount;
	uintsize				keyCapacity;

	uintsize				keyOffset;
	uintsize				keySize;
};

bool			ImUiHashTableCreateSize( ImUiAllocator* allocator, ImUiHashTable* hashTable, uintsize keyOffset, uintsize keySize, uintsize initialSize );
bool			ImUiHashTableCreateStatic( ImUiAllocator* allocator, ImUiHashTable* hashTable, const void* data, uintsize elementSize, uintsize elementCount, uintsize keyOffset, uintsize keySize );
void			ImUiHashTableDestroy( ImUiHashTable* hashTable );

const uint32*	ImUiHashTableFind( ImUiHashTable* hashTable, const void* element );

bool			ImUiHashTableInsert( ImUiHashTable* hashTable, const void* element, uint32 index );
bool			ImUiHashTableRemove( ImUiHashTable* hashTable, const void* element );

typedef struct ImUiStringPool ImUiStringPool;
struct ImUiStringPool
{
	ImUiAllocator*			allocator;
	ImUiStringPoolChunk*	firstChunk;

	ImUiHashTable			map;
	ImUiStringPoolKey*		keys;
	uintsize				keyCount;
	uintsize				keyCapacity;
};

void						ImUiStringPoolConstruct( ImUiStringPool* stringPool, ImUiAllocator* allocator );
void						ImUiStringPoolDestruct( ImUiStringPool* stringPool );

void						ImUiStringPoolClear( ImUiStringPool* stringPool );

ImUiStringView				ImUiStringPoolAdd( ImUiStringPool* stringPool, ImUiStringView string );
ImUiStringView				ImUiStringPoolFindByHash( ImUiStringPool* stringPool, ImUiHash stringHash );

