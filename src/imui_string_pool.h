#pragma once

typedef struct ImUiAllocator ImUiAllocator;
typedef struct ImUiStringPoolChunk ImUiStringPoolChunk;
typedef struct ImUiStringPoolKey ImUiStringPoolKey;

typedef struct ImUiStringPool ImUiStringPool;
struct ImUiStringPool
{
	ImUiAllocator*			allocator;
	ImUiStringPoolChunk*	firstChunk;

	ImUiStringPoolKey*		keys;
	uintsize				keyCount;
	uintsize				keyCapacity;
};

void			ImUiStringPoolConstruct( ImUiStringPool* stringPool, ImUiAllocator* allocator );
void			ImUiStringPoolDestruct( ImUiStringPool* stringPool );

void			ImUiStringPoolClear( ImUiStringPool* stringPool );

ImUiStringView	ImUiStringPoolAdd( ImUiStringPool* stringPool, ImUiStringView string );
ImUiStringView	ImUiStringPoolFindByHash( ImUiStringPool* stringPool, ImUiHash stringHash );
