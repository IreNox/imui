#include "imui_helpers.h"

#include "imui_internal.h"
#include "imui_memory.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
// HashMap

bool imuiHashMapConstructSize( ImuiHashMap* hashMap, ImuiAllocator* allocator, uintsize entrySize, imuiHashMapEntryHashFunc entryHashFunc, imuiHashMapIsKeyEqualsFunc entryKeyEqualsFunc, uintsize initialSize )
{
	// round up to next power of 2
	initialSize = IMUI_MAX( initialSize, 2u );
	initialSize--;
	initialSize |= initialSize >> 1;
	initialSize |= initialSize >> 2;
	initialSize |= initialSize >> 4;
	initialSize |= initialSize >> 8;
	initialSize |= initialSize >> 16;
	initialSize++;

	const uintsize entriesInUseCount = (initialSize + 64u - 1) & (0 - 64);

	hashMap->allocator			= allocator;
	hashMap->entriesInUse		= IMUI_MEMORY_ARRAY_NEW_ZERO( allocator, uint64, entriesInUseCount >> 6u );
	hashMap->entries			= (uint8*)imuiMemoryAlloc( allocator, entrySize * initialSize );
	hashMap->entryCount			= 0u;
	hashMap->entryCapacity		= initialSize;
	hashMap->entrySize			= entrySize;
	hashMap->entryHashFunc		= entryHashFunc;
	hashMap->entryKeyEqualsFunc	= entryKeyEqualsFunc;

	if( !hashMap->entriesInUse || !hashMap->entries )
	{
		imuiHashMapDestruct( hashMap );
		return false;
	}

	return true;
}

bool imuiHashMapConstructStatic( ImuiHashMap* hashMap, ImuiAllocator* allocator, const void* data, uintsize entrySize, uintsize entryCount, imuiHashMapEntryHashFunc entryHashFunc, imuiHashMapIsKeyEqualsFunc entryKeyEqualsFunc )
{
	if( !imuiHashMapConstructSize( hashMap, allocator, entrySize, entryHashFunc, entryKeyEqualsFunc, entryCount * 2u ) )
	{
		return false;
	}

	const uint8* element = (const uint8*)data;
	for( uint32 i = 0; i < entryCount; ++i )
	{
		if( !imuiHashMapInsert( hashMap, element ) )
		{
			imuiHashMapDestruct( hashMap );
			return false;
		}

		element += entrySize;
	}

	return true;
}

bool imuiHashMapConstructStaticPointer( ImuiHashMap* hashMap, ImuiAllocator* allocator, const void* data, uintsize entrySize, uintsize entryCount, imuiHashMapEntryHashFunc entryHashFunc, imuiHashMapIsKeyEqualsFunc entryKeyEqualsFunc )
{
	if( !imuiHashMapConstructSize( hashMap, allocator, sizeof( void* ), entryHashFunc, entryKeyEqualsFunc, entryCount * 2u) )
	{
		return false;
	}

	const uint8* element = (const uint8*)data;
	for( uint32 i = 0; i < entryCount; ++i )
	{
		if( !imuiHashMapInsert( hashMap, &element ) )
		{
			imuiHashMapDestruct( hashMap );
			return false;
		}

		element += entrySize;
	}

	return true;
}

void imuiHashMapDestruct( ImuiHashMap* hashMap )
{
	imuiMemoryFree( hashMap->allocator, hashMap->entriesInUse );
	imuiMemoryFree( hashMap->allocator, hashMap->entries );

	hashMap->entriesInUse			= NULL;
	hashMap->entries			= NULL;
	hashMap->entryCount			= 0u;
	hashMap->entryCapacity		= 0u;
	hashMap->entrySize			= 0u;
	hashMap->entryHashFunc		= NULL;
	hashMap->entryKeyEqualsFunc	= NULL;
	hashMap->allocator			= NULL;
}

static bool imuiHashMapGrow( ImuiHashMap* hashMap )
{
	uintsize nextCapacity = hashMap->entryCapacity;
	uint64* newEntriesInUse;
	uint8* newEntries;
	bool retry = false;
	do
	{
		nextCapacity <<= 1u;

		const uint32 nextIndexMask = (uint32)nextCapacity - 1u;

		newEntriesInUse = IMUI_MEMORY_ARRAY_NEW_ZERO( hashMap->allocator, uint64, nextCapacity >> 6u );
		newEntries = (uint8*)imuiMemoryAlloc( hashMap->allocator, nextCapacity * hashMap->entrySize );
		if( !newEntriesInUse || !newEntries )
		{
			return false;
		}

		for( uintsize mapIndex = 0; mapIndex < hashMap->entryCapacity; ++mapIndex )
		{
			const uint64* mapEntryInUse = &hashMap->entriesInUse[ mapIndex >> 6u ];
			const uint64 mapEntryInUseMask = 1ull << (mapIndex & 0x3fu);
			if( (*mapEntryInUse & mapEntryInUseMask) == 0u )
			{
				continue;
			}

			const uint8* mapEntry = &hashMap->entries[ mapIndex * hashMap->entrySize ];
			const ImuiHash hash = hashMap->entryHashFunc( mapEntry );

			for( uint32 hashOffset = 0; ; ++hashOffset )
			{
				const uint32 newIndex = (hash + hashOffset) & nextIndexMask;

				uint64* newEntryInUse = &newEntriesInUse[ newIndex >> 6u ];
				const uint64 newEntryInUseMask = 1ull << (newIndex & 0x3fu);
				if( (*newEntryInUse & newEntryInUseMask) != 0u )
				{
					const uint8* newEntry = &newEntries[ newIndex * hashMap->entrySize ];
					const ImuiHash newHash = hashMap->entryHashFunc( newEntry );
					if( (newHash & nextIndexMask) != (hash & nextIndexMask) )
					{
						retry = true;
						break;
					}
					continue;
				}

				*newEntryInUse |= newEntryInUseMask;

				uint8* newEntry = &newEntries[ newIndex * hashMap->entrySize ];
				memcpy( newEntry, mapEntry, hashMap->entrySize );
				break;
			}

			if( retry )
			{
				imuiMemoryFree( hashMap->allocator, newEntriesInUse );
				imuiMemoryFree( hashMap->allocator, newEntries );
				break;
			}
		}
	}
	while( retry );

	imuiMemoryFree( hashMap->allocator, hashMap->entriesInUse );
	imuiMemoryFree( hashMap->allocator, hashMap->entries );

	hashMap->entriesInUse	= newEntriesInUse;
	hashMap->entries		= newEntries;
	hashMap->entryCapacity	= nextCapacity;

	return true;
}

void* imuiHashMapFind( ImuiHashMap* hashMap, const void* entry )
{
	const ImuiHash hash		= hashMap->entryHashFunc( entry );
	const uint32 indexMask	= (uint32)hashMap->entryCapacity - 1u;

	for( uint32 hashOffset = 0u; ; ++hashOffset )
	{
		const uintsize index = (hash + hashOffset) & indexMask;

		uint64* mapEntryInUse = &hashMap->entriesInUse[ index >> 6u ];
		const uint64 mapEntryInUseMask = 1ull << (index & 0x3fu);
		if( (*mapEntryInUse & mapEntryInUseMask) == 0u )
		{
			break;
		}

		uint8* mapEntry = &hashMap->entries[ index * hashMap->entrySize ];
		if( hashMap->entryKeyEqualsFunc( mapEntry, entry ) )
		{
			return mapEntry;
		}

		const ImuiHash mapHash = hashMap->entryHashFunc( mapEntry );
		if( (mapHash & indexMask) != (hash & indexMask) )
		{
			break;
		}
	}

	return NULL;
}

void* imuiHashMapInsert( ImuiHashMap* hashMap, const void* entry )
{
	return imuiHashMapInsertNew( hashMap, entry, NULL );
}

void* imuiHashMapInsertNew( ImuiHashMap* hashMap, const void* entry, bool* isNew )
{
	const ImuiHash hash		= hashMap->entryHashFunc( entry );
	uint32 indexMask		= (uint32)hashMap->entryCapacity - 1u;

	uint32 hashOffset = 0;
	while( true )
	{
		const uint32 index = (hash + hashOffset) & indexMask;

		uint8* mapEntry = &hashMap->entries[ index * hashMap->entrySize ];

		uint64* mapEntryInUse = &hashMap->entriesInUse[ index >> 6u ];
		const uint64 mapEntryInUseMask = 1ull << (index & 0x3fu);
		if( (*mapEntryInUse & mapEntryInUseMask) == 0u )
		{
			*mapEntryInUse |= mapEntryInUseMask;

			memcpy( mapEntry, entry, hashMap->entrySize );

			hashMap->entryCount++;

			if( isNew )
			{
				*isNew = true;
			}

			return mapEntry;
		}

		if( hashMap->entryKeyEqualsFunc( mapEntry, entry ) )
		{
			if( isNew )
			{
				*isNew = false;
			}

			return mapEntry;
		}

		hashOffset++;

		const ImuiHash mapHash = hashMap->entryHashFunc( mapEntry );
		if( (mapHash & indexMask) != (hash & indexMask) )
		{
			if( !imuiHashMapGrow( hashMap ) )
			{
				break;
			}

			hashOffset = 0u;
			indexMask = (uint32)hashMap->entryCapacity - 1u;
		}
	}

	return NULL;
}

bool imuiHashMapRemove( ImuiHashMap* hashMap, const void* entry )
{
	const ImuiHash hash		= hashMap->entryHashFunc( entry );
	const uint32 indexMask	= (uint32)hashMap->entryCapacity - 1u;

	for( uint32 hashOffset = 0u; ; ++hashOffset )
	{
		const uint32 index = (hash + hashOffset) & indexMask;

		uint64* mapEntryInUse = &hashMap->entriesInUse[ index >> 6u ];
		uint64 mapEntryInUseMask = 1ull << (index & 0x3fu);
		if( (*mapEntryInUse & mapEntryInUseMask) == 0u )
		{
			break;
		}

		uint8* mapEntry = &hashMap->entries[ index * hashMap->entrySize ];
		if( hashMap->entryKeyEqualsFunc( mapEntry, entry ) )
		{
			const uint32 baseIndex = hash & indexMask;

			uint32 nextIndex = index;
			while( true )
			{
				nextIndex = (nextIndex + 1u) & indexMask;

				uint64* nextMapEntryInUse = &hashMap->entriesInUse[ nextIndex >> 6u ];
				const uint64 nextMapEntryInUseMask = 1ull << (nextIndex & 0x3fu);
				if( (*nextMapEntryInUse & nextMapEntryInUseMask) == 0u )
				{
					break;
				}

				uint8* nextMapEntry = &hashMap->entries[ nextIndex * hashMap->entrySize ];
				const ImuiHash nextHash = hashMap->entryHashFunc( nextMapEntry );
				if( (nextHash & indexMask) != baseIndex )
				{
					break;
				}

				memcpy( mapEntry, nextMapEntry, hashMap->entrySize );

				mapEntry = nextMapEntry;
				mapEntryInUse = nextMapEntryInUse;
				mapEntryInUseMask = nextMapEntryInUseMask;
			}

			*mapEntryInUse &= ~mapEntryInUseMask;
			hashMap->entryCount--;

			memset( mapEntry, 0, hashMap->entrySize );

			return true;
		}
	}

	return false;
}

uintsize imuiHashMapFindFirstIndex( ImuiHashMap* hashMap )
{
	if( hashMap->entryCount == 0u )
	{
		return IMUI_SIZE_MAX;
	}

	const uint64* mapEntryInUse = hashMap->entriesInUse;
	uint64 mapEntryInUseMask = 1u;
	for( uintsize mapIndex = 0u; mapIndex < hashMap->entryCapacity; ++mapIndex )
	{
		if( *mapEntryInUse & mapEntryInUseMask )
		{
			return mapIndex;
		}

		mapEntryInUseMask <<= 1u;
		if( mapEntryInUseMask == 0u )
		{
			mapEntryInUse++;
			mapEntryInUseMask = 1u;
		}
	}

	return IMUI_SIZE_MAX;
}

uintsize imuiHashMapFindNextIndex( ImuiHashMap* hashMap, uintsize mapIndex )
{
	++mapIndex;

	const uint64* mapEntryInUse = &hashMap->entriesInUse[ mapIndex >> 6u ];
	uint64 mapEntryInUseMask = 1ull << (mapIndex & 0x3fu);

	for( ; mapIndex < hashMap->entryCapacity; ++mapIndex )
	{
		if( *mapEntryInUse & mapEntryInUseMask )
		{
			return mapIndex;
		}

		mapEntryInUseMask <<= 1u;
		if( mapEntryInUseMask == 0u )
		{
			mapEntryInUse++;
			mapEntryInUseMask = 1u;
		}
	}

	return IMUI_SIZE_MAX;
}

void* imuiHashMapGetEntry( ImuiHashMap* hashMap, uintsize index )
{
	return &hashMap->entries[ index * hashMap->entrySize ];
}

//////////////////////////////////////////////////////////////////////////
// String Pool

struct ImuiStringPoolChunk
{
	ImuiStringPoolChunk*	nextChunk;

	uintsize				usedSize;
	uintsize				remainingSize;
	char					data[ 1u ];
};

static ImuiHash imuiStringPoolHash( const void* entry )
{
	const ImuiStringView* string = (const ImuiStringView*)entry;
	return imuiHashString( *string );
}

static bool imuiStringPoolIsKeyEquals( const void* lhs, const void* rhs )
{
	const ImuiStringView* lhsString = (const ImuiStringView*)lhs;
	const ImuiStringView* rhsString = (const ImuiStringView*)rhs;

	if( lhsString->length != rhsString->length )
	{
		return false;
	}
	else if( lhsString->length == 0u )
	{
		return true;
	}
	else if( lhsString->data[ 0u ] != rhsString->data[ 0u ] )
	{
		return false;
	}

	return memcmp( lhsString->data, rhsString->data, lhsString->length ) == 0;
}

bool imuiStringPoolConstruct( ImuiStringPool* stringPool, ImuiAllocator* allocator )
{
	stringPool->allocator	= allocator;
	stringPool->firstChunk	= NULL;

	if( !imuiHashMapConstructSize( &stringPool->keyMap, allocator, sizeof( ImuiStringView ), imuiStringPoolHash, imuiStringPoolIsKeyEquals, 64u ) )
	{
		imuiStringPoolDestruct( stringPool );
		return false;
	}

	return true;
}

void imuiStringPoolDestruct( ImuiStringPool* stringPool )
{
	imuiHashMapDestruct( &stringPool->keyMap );

	ImuiStringPoolChunk* chunk = stringPool->firstChunk;
	ImuiStringPoolChunk* nextChunk = NULL;
	while( chunk )
	{
		nextChunk = chunk->nextChunk;
		imuiMemoryFree( stringPool->allocator, chunk );
		chunk = nextChunk;
	}

	stringPool->firstChunk	= NULL;
	stringPool->allocator	= NULL;
}

void imuiStringPoolClear( ImuiStringPool* stringPool )
{
	for( ImuiStringPoolChunk* chunk = stringPool->firstChunk; chunk != NULL; chunk = chunk->nextChunk )
	{
		chunk->remainingSize += chunk->usedSize;
		chunk->usedSize = 0u;
	}
}

ImuiStringView imuiStringPoolAdd( ImuiStringPool* stringPool, ImuiStringView string )
{
	bool isNew;
	ImuiStringView* mapEntry = (ImuiStringView*)imuiHashMapInsertNew( &stringPool->keyMap, &string, &isNew );
	if( !mapEntry )
	{
		return imuiStringViewCreateEmpty();
	}
	else if( !isNew )
	{
		return *mapEntry;
	}

	ImuiStringPoolChunk* chunk;
	if( stringPool->firstChunk == NULL ||
		string.length >= stringPool->firstChunk->remainingSize )
	{
		uintsize size = IMUI_DEFAULT_STRING_POOL_CHUNK_SIZE;
		if( string.length >= size )
		{
			size = string.length;
		}

		chunk = (ImuiStringPoolChunk*)imuiMemoryAlloc( stringPool->allocator, sizeof( ImuiStringPoolChunk ) + size );
		if( !chunk )
		{
			return imuiStringViewCreateEmpty();
		}

		chunk->nextChunk		= stringPool->firstChunk;
		chunk->usedSize			= 0u;
		chunk->remainingSize	= size + 1u;

		stringPool->firstChunk = chunk;
	}
	else
	{
		chunk = stringPool->firstChunk;
	}

	char* target = chunk->data + chunk->usedSize;
	memcpy( target, string.data, string.length );
	target[ string.length ] = '\0';

	chunk->usedSize += string.length + 1u;
	chunk->remainingSize -= string.length + 1u;

	mapEntry->data		= target;
	mapEntry->length	= string.length;

	return *mapEntry;
}

const ImuiStringView* imuiStringPoolFind( ImuiStringPool* stringPool, ImuiStringView string )
{
	return (const ImuiStringView*)imuiHashMapFind( &stringPool->keyMap, &string );
}
