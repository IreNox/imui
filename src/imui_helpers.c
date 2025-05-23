#include "imui_helpers.h"

#include "imui_internal.h"
#include "imui_memory.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
// HashMap

bool ImUiHashMapConstructSize( ImUiHashMap* hashMap, ImUiAllocator* allocator, uintsize entrySize, ImUiHashMapEntryHashFunc entryHashFunc, ImUiHashMapIsKeyEqualsFunc entryKeyEqualsFunc, uintsize initialSize )
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
	hashMap->entries			= (uint8*)ImUiMemoryAlloc( allocator, entrySize * initialSize );
	hashMap->entryCount			= 0u;
	hashMap->entryCapacity		= initialSize;
	hashMap->entrySize			= entrySize;
	hashMap->entryHashFunc		= entryHashFunc;
	hashMap->entryKeyEqualsFunc	= entryKeyEqualsFunc;

	if( !hashMap->entriesInUse || !hashMap->entries )
	{
		ImUiHashMapDestruct( hashMap );
		return false;
	}

	return true;
}

bool ImUiHashMapConstructStatic( ImUiHashMap* hashMap, ImUiAllocator* allocator, const void* data, uintsize entrySize, uintsize entryCount, ImUiHashMapEntryHashFunc entryHashFunc, ImUiHashMapIsKeyEqualsFunc entryKeyEqualsFunc )
{
	if( !ImUiHashMapConstructSize( hashMap, allocator, entrySize, entryHashFunc, entryKeyEqualsFunc, entryCount * 2u ) )
	{
		return false;
	}

	const uint8* element = (const uint8*)data;
	for( uint32 i = 0; i < entryCount; ++i )
	{
		if( !ImUiHashMapInsert( hashMap, element ) )
		{
			ImUiHashMapDestruct( hashMap );
			return false;
		}

		element += entrySize;
	}

	return true;
}

bool ImUiHashMapConstructStaticPointer( ImUiHashMap* hashMap, ImUiAllocator* allocator, const void* data, uintsize entrySize, uintsize entryCount, ImUiHashMapEntryHashFunc entryHashFunc, ImUiHashMapIsKeyEqualsFunc entryKeyEqualsFunc )
{
	if( !ImUiHashMapConstructSize( hashMap, allocator, sizeof( void* ), entryHashFunc, entryKeyEqualsFunc, entryCount * 2u) )
	{
		return false;
	}

	const uint8* element = (const uint8*)data;
	for( uint32 i = 0; i < entryCount; ++i )
	{
		if( !ImUiHashMapInsert( hashMap, &element ) )
		{
			ImUiHashMapDestruct( hashMap );
			return false;
		}

		element += entrySize;
	}

	return true;
}

void ImUiHashMapDestruct( ImUiHashMap* hashMap )
{
	ImUiMemoryFree( hashMap->allocator, hashMap->entriesInUse );
	ImUiMemoryFree( hashMap->allocator, hashMap->entries );

	hashMap->entriesInUse			= NULL;
	hashMap->entries			= NULL;
	hashMap->entryCount			= 0u;
	hashMap->entryCapacity		= 0u;
	hashMap->entrySize			= 0u;
	hashMap->entryHashFunc		= NULL;
	hashMap->entryKeyEqualsFunc	= NULL;
	hashMap->allocator			= NULL;
}

static bool ImUiHashMapGrow( ImUiHashMap* hashMap )
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
		newEntries = (uint8*)ImUiMemoryAlloc( hashMap->allocator, nextCapacity * hashMap->entrySize );
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
			const ImUiHash hash = hashMap->entryHashFunc( mapEntry );

			for( uint32 hashOffset = 0; ; ++hashOffset )
			{
				const uint32 newIndex = (hash + hashOffset) & nextIndexMask;

				uint64* newEntryInUse = &newEntriesInUse[ newIndex >> 6u ];
				const uint64 newEntryInUseMask = 1ull << (newIndex & 0x3fu);
				if( (*newEntryInUse & newEntryInUseMask) != 0u )
				{
					const uint8* newEntry = &newEntries[ newIndex * hashMap->entrySize ];
					const ImUiHash newHash = hashMap->entryHashFunc( newEntry );
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
				ImUiMemoryFree( hashMap->allocator, newEntriesInUse );
				ImUiMemoryFree( hashMap->allocator, newEntries );
				break;
			}
		}
	}
	while( retry );

	ImUiMemoryFree( hashMap->allocator, hashMap->entriesInUse );
	ImUiMemoryFree( hashMap->allocator, hashMap->entries );

	hashMap->entriesInUse	= newEntriesInUse;
	hashMap->entries		= newEntries;
	hashMap->entryCapacity	= nextCapacity;

	return true;
}

void* ImUiHashMapFind( ImUiHashMap* hashMap, const void* entry )
{
	const ImUiHash hash		= hashMap->entryHashFunc( entry );
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

		const ImUiHash mapHash = hashMap->entryHashFunc( mapEntry );
		if( (mapHash & indexMask) != (hash & indexMask) )
		{
			break;
		}
	}

	return NULL;
}

void* ImUiHashMapInsert( ImUiHashMap* hashMap, const void* entry )
{
	return ImUiHashMapInsertNew( hashMap, entry, NULL );
}

void* ImUiHashMapInsertNew( ImUiHashMap* hashMap, const void* entry, bool* isNew )
{
	const ImUiHash hash		= hashMap->entryHashFunc( entry );
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

		const ImUiHash mapHash = hashMap->entryHashFunc( mapEntry );
		if( (mapHash & indexMask) != (hash & indexMask) )
		{
			if( !ImUiHashMapGrow( hashMap ) )
			{
				break;
			}

			hashOffset = 0u;
			indexMask = (uint32)hashMap->entryCapacity - 1u;
		}
	}

	return NULL;
}

bool ImUiHashMapRemove( ImUiHashMap* hashMap, const void* entry )
{
	const ImUiHash hash		= hashMap->entryHashFunc( entry );
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
				const ImUiHash nextHash = hashMap->entryHashFunc( nextMapEntry );
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

uintsize ImUiHashMapFindFirstIndex( ImUiHashMap* hashMap )
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

uintsize ImUiHashMapFindNextIndex( ImUiHashMap* hashMap, uintsize mapIndex )
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

void* ImUiHashMapGetEntry( ImUiHashMap* hashMap, uintsize index )
{
	return &hashMap->entries[ index * hashMap->entrySize ];
}

//////////////////////////////////////////////////////////////////////////
// String Pool

struct ImUiStringPoolChunk
{
	ImUiStringPoolChunk*	nextChunk;

	uintsize				usedSize;
	uintsize				remainingSize;
	char					data[ 1u ];
};

static ImUiHash ImUiStringPoolHash( const void* entry )
{
	const ImUiStringView* string = (const ImUiStringView*)entry;
	return ImUiHashString( *string );
}

static bool ImUiStringPoolIsKeyEquals( const void* lhs, const void* rhs )
{
	const ImUiStringView* lhsString = (const ImUiStringView*)lhs;
	const ImUiStringView* rhsString = (const ImUiStringView*)rhs;

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

bool ImUiStringPoolConstruct( ImUiStringPool* stringPool, ImUiAllocator* allocator )
{
	stringPool->allocator	= allocator;
	stringPool->firstChunk	= NULL;

	if( !ImUiHashMapConstructSize( &stringPool->keyMap, allocator, sizeof( ImUiStringView ), ImUiStringPoolHash, ImUiStringPoolIsKeyEquals, 64u ) )
	{
		ImUiStringPoolDestruct( stringPool );
		return false;
	}

	return true;
}

void ImUiStringPoolDestruct( ImUiStringPool* stringPool )
{
	ImUiHashMapDestruct( &stringPool->keyMap );

	ImUiStringPoolChunk* chunk = stringPool->firstChunk;
	ImUiStringPoolChunk* nextChunk = NULL;
	while( chunk )
	{
		nextChunk = chunk->nextChunk;
		ImUiMemoryFree( stringPool->allocator, chunk );
		chunk = nextChunk;
	}

	stringPool->firstChunk	= NULL;
	stringPool->allocator	= NULL;
}

void ImUiStringPoolClear( ImUiStringPool* stringPool )
{
	for( ImUiStringPoolChunk* chunk = stringPool->firstChunk; chunk != NULL; chunk = chunk->nextChunk )
	{
		chunk->remainingSize += chunk->usedSize;
		chunk->usedSize = 0u;
	}
}

ImUiStringView ImUiStringPoolAdd( ImUiStringPool* stringPool, ImUiStringView string )
{
	bool isNew;
	ImUiStringView* mapEntry = (ImUiStringView*)ImUiHashMapInsertNew( &stringPool->keyMap, &string, &isNew );
	if( !mapEntry )
	{
		return ImUiStringViewCreateEmpty();
	}
	else if( !isNew )
	{
		return *mapEntry;
	}

	ImUiStringPoolChunk* chunk;
	if( stringPool->firstChunk == NULL ||
		string.length >= stringPool->firstChunk->remainingSize )
	{
		uintsize size = IMUI_DEFAULT_STRING_POOL_CHUNK_SIZE;
		if( string.length >= size )
		{
			size = string.length;
		}

		chunk = (ImUiStringPoolChunk*)ImUiMemoryAlloc( stringPool->allocator, sizeof( ImUiStringPoolChunk ) + size );
		if( !chunk )
		{
			return ImUiStringViewCreateEmpty();
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

const ImUiStringView* ImUiStringPoolFind( ImUiStringPool* stringPool, ImUiStringView string )
{
	return (const ImUiStringView*)ImUiHashMapFind( &stringPool->keyMap, &string );
}
