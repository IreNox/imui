#pragma once

typedef struct ImUiAllocator ImUiAllocator;
typedef struct ImUiStringPoolChunk ImUiStringPoolChunk;

typedef struct ImUiStringPool ImUiStringPool;
struct ImUiStringPool
{
	ImUiAllocator*			allocator;
	ImUiStringPoolChunk*	firstChunk;
};

void			ImUiStringPoolConstruct( ImUiStringPool* stringPool, ImUiAllocator* allocator );
void			ImUiStringPoolDestruct( ImUiStringPool* stringPool );

void			ImUiStringPoolClear( ImUiStringPool* stringPool );

ImUiStringView	ImUiStringPoolAdd( ImUiStringPool* stringPool, ImUiStringView string );
