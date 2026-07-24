#pragma once

#include "imui/imui.h"
#include "imui_types.h"

typedef ImuiHash(*imuiHashMapEntryHashFunc)( const void* entry );
typedef bool(*imuiHashMapIsKeyEqualsFunc)( const void* lhs, const void* rhs );

typedef struct ImuiHashMap
{
	ImuiAllocator*				allocator;

	uint64*						entriesInUse;
	uint8*						entries;
	uintsize					entryCount;
	uintsize					entryCapacity;

	uintsize					entrySize;
	imuiHashMapEntryHashFunc	entryHashFunc;
	imuiHashMapIsKeyEqualsFunc	entryKeyEqualsFunc;
} ImuiHashMap;

bool			imuiHashMapConstructSize( ImuiHashMap* hashMap, ImuiAllocator* allocator, uintsize entrySize, imuiHashMapEntryHashFunc entryHashFunc, imuiHashMapIsKeyEqualsFunc entryKeyEqualsFunc, uintsize initialSize );
bool			imuiHashMapConstructStatic( ImuiHashMap* hashMap, ImuiAllocator* allocator, const void* data, uintsize entrySize, uintsize entryCount, imuiHashMapEntryHashFunc entryHashFunc, imuiHashMapIsKeyEqualsFunc entryKeyEqualsFunc );
bool			imuiHashMapConstructStaticPointer( ImuiHashMap* hashMap, ImuiAllocator* allocator, const void* data, uintsize entrySize, uintsize entryCount, imuiHashMapEntryHashFunc entryHashFunc, imuiHashMapIsKeyEqualsFunc entryKeyEqualsFunc );
void			imuiHashMapDestruct( ImuiHashMap* hashMap );

void*			imuiHashMapFind( ImuiHashMap* hashMap, const void* entry );

void*			imuiHashMapInsert( ImuiHashMap* hashMap, const void* entry );
void*			imuiHashMapInsertNew( ImuiHashMap* hashMap, const void* entry, bool* isNew );
bool			imuiHashMapRemove( ImuiHashMap* hashMap, const void* entry );

uintsize		imuiHashMapFindFirstIndex( ImuiHashMap* hashMap );
uintsize		imuiHashMapFindNextIndex( ImuiHashMap* hashMap, uintsize entry );

void*			imuiHashMapGetEntry( ImuiHashMap* hashMap, uintsize index );

typedef struct ImuiStringPoolChunk ImuiStringPoolChunk;

typedef struct ImuiStringPool
{
	ImuiAllocator*			allocator;
	ImuiStringPoolChunk*	firstChunk;

	ImuiHashMap				keyMap;
} ImuiStringPool;

bool						imuiStringPoolConstruct( ImuiStringPool* stringPool, ImuiAllocator* allocator );
void						imuiStringPoolDestruct( ImuiStringPool* stringPool );

void						imuiStringPoolClear( ImuiStringPool* stringPool );

ImuiStringView				imuiStringPoolAdd( ImuiStringPool* stringPool, ImuiStringView string );
const ImuiStringView*		imuiStringPoolFind( ImuiStringPool* stringPool, ImuiStringView string );

