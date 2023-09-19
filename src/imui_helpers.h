#pragma once

#include "imui/imui.h"
#include "imui_types.h"

typedef struct ImUiAllocator ImUiAllocator;

typedef ImUiHash(*ImUiHashMapEntryHashFunc)( const void* entry );
typedef bool(*ImUiHashMapIsKeyEqualsFunc)( const void* lhs, const void* rhs );

typedef struct ImUiHashMap ImUiHashMap;
struct ImUiHashMap
{
	ImUiAllocator*				allocator;

	uint64*						entriesInUse;
	uint8*						entries;
	uintsize					entryCount;
	uintsize					entryCapacity;

	uintsize					entrySize;
	ImUiHashMapEntryHashFunc	entryHashFunc;
	ImUiHashMapIsKeyEqualsFunc	entryKeyEqualsFunc;
};

bool			ImUiHashMapConstructSize( ImUiHashMap* hashMap, ImUiAllocator* allocator, uintsize entrySize, ImUiHashMapEntryHashFunc entryHashFunc, ImUiHashMapIsKeyEqualsFunc entryKeyEqualsFunc, uintsize initialSize );
bool			ImUiHashMapConstructStatic( ImUiHashMap* hashMap, ImUiAllocator* allocator, const void* data, uintsize entrySize, uintsize entryCount, ImUiHashMapEntryHashFunc entryHashFunc, ImUiHashMapIsKeyEqualsFunc entryKeyEqualsFunc );
bool			ImUiHashMapConstructStaticPointer( ImUiHashMap* hashMap, ImUiAllocator* allocator, const void* data, uintsize entrySize, uintsize entryCount, ImUiHashMapEntryHashFunc entryHashFunc, ImUiHashMapIsKeyEqualsFunc entryKeyEqualsFunc );
void			ImUiHashMapDestruct( ImUiHashMap* hashMap );

void*			ImUiHashMapFind( ImUiHashMap* hashMap, const void* entry );

void*			ImUiHashMapInsert( ImUiHashMap* hashMap, const void* entry );
void*			ImUiHashMapInsertNew( ImUiHashMap* hashMap, const void* entry, bool* isNew );
bool			ImUiHashMapRemove( ImUiHashMap* hashMap, const void* entry );

uintsize		ImUiHashMapFindFirstIndex( ImUiHashMap* hashMap );
uintsize		ImUiHashMapFindNextIndex( ImUiHashMap* hashMap, uintsize entry );

void*			ImUiHashMapGetEntry( ImUiHashMap* hashMap, uintsize index );

typedef struct ImUiStringPoolChunk ImUiStringPoolChunk;
typedef struct ImUiStringPoolKey ImUiStringPoolKey;

typedef struct ImUiStringPool ImUiStringPool;
struct ImUiStringPool
{
	ImUiAllocator*			allocator;
	ImUiStringPoolChunk*	firstChunk;

	ImUiHashMap				keyMap;
};

bool						ImUiStringPoolConstruct( ImUiStringPool* stringPool, ImUiAllocator* allocator );
void						ImUiStringPoolDestruct( ImUiStringPool* stringPool );

void						ImUiStringPoolClear( ImUiStringPool* stringPool );

ImUiStringView				ImUiStringPoolAdd( ImUiStringPool* stringPool, ImUiStringView string );
const ImUiStringView*		ImUiStringPoolFind( ImUiStringPool* stringPool, ImUiStringView string );

