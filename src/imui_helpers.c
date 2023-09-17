#include "imui_helpers.h"

#include "imui_internal.h"
#include "imui_memory.h"

#include <string.h>


//////////////////////////////////////////////////////////////////////////
// Chunked Pool

struct ImUiChunkedPoolChunk
{
	uintsize	remainingElements;
	uint8		data[ 1u ];
};

struct ImUiChunkedPoolFreeElement
{
	ImUiChunkedPoolFreeElement*	nextElement;
};

bool ImUiChunkedPoolConstruct( ImUiChunkedPool* pool, ImUiAllocator* allocator, uintsize elementSize, uintsize chunkSize )
{
	IMUI_ASSERT( (chunkSize & (chunkSize - 1u)) == 0u ); // chunkSize must be power of 2
	IMUI_ASSERT( elementSize >= sizeof( ImUiChunkedPoolFreeElement ) );

	pool->allocator			= allocator;
	pool->elementSize		= elementSize;
	pool->chunkSize			= chunkSize;
	pool->chunks			= NULL;
	pool->chunkCount		= 0u;
	pool->chunkCapacity		= 0u;
	pool->firstFreeElement	= NULL;

	return true;
}

void ImUiChunkedPoolDestruct( ImUiChunkedPool* pool )
{
	for( uintsize i = 0; i < pool->chunkCount; ++i )
	{
		ImUiMemoryFree( pool->allocator, pool->chunks[ i ] );
	}
	ImUiMemoryFree( pool->allocator, pool->chunks );

	pool->chunks		= NULL;
	pool->chunkCount	= 0u;
	pool->chunkCapacity	= 0u;
	pool->allocator		= NULL;
}

void* ImUiChunkedPoolAllocate( ImUiChunkedPool* pool )
{
	if( pool->firstFreeElement )
	{
		ImUiChunkedPoolFreeElement* freeElement = pool->firstFreeElement;
		pool->firstFreeElement = freeElement->nextElement;

		return freeElement;
	}

	ImUiChunkedPoolChunk* chunk = NULL;
	if( pool->chunks == NULL ||
		pool->chunks[ pool->chunkCount - 1u ]->remainingElements == 0u )
	{
		if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY( pool->allocator, pool->chunks, pool->chunkCapacity, pool->chunkCount + 1u ) )
		{
			return NULL;
		}

		chunk = (ImUiChunkedPoolChunk*)ImUiMemoryAlloc( pool->allocator, sizeof( ImUiChunkedPoolChunk ) + (pool->elementSize * pool->chunkSize) );
		chunk->remainingElements = pool->chunkSize;

		pool->chunks[ pool->chunkCount ] = chunk;
		pool->chunkCount++;
	}
	else
	{
		chunk = pool->chunks[ pool->chunkCount - 1u ];
	}

	const uintsize elementIndex = pool->chunkSize - chunk->remainingElements;
	chunk->remainingElements--;

	return &chunk->data[ elementIndex * pool->elementSize ];
}

void ImUiChunkedPoolFree( ImUiChunkedPool* pool, void* element )
{
	ImUiChunkedPoolFreeElement* freeElement = (ImUiChunkedPoolFreeElement*)element;
	freeElement->nextElement = pool->firstFreeElement;

	pool->firstFreeElement = freeElement;
}

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

	const uintsize entriesInUseCount = (initialSize + 64u - 1) & (0 - 64u);

	hashMap->allocator			= allocator;
	hashMap->entriesInUse		= IMUI_MEMORY_ARRAY_NEW_ZERO( allocator, uint64, entriesInUseCount );
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

static uintsize ImUiHashMapFindInternal( ImUiHashMap* hashMap, const void* entry )
{
	for( uint32 hashOffset = 0u; hashOffset < 4u; ++hashOffset )
	{
		const ImUiHash hash = hashMap->entryHashFunc( entry );
		const uintsize index = (hash + hashOffset) & (hashMap->entryCapacity - 1u);

		uint64* mapEntryInUse = &hashMap->entriesInUse[ index >> 6u ];
		const uint64 mapEntryInUseMask = 1ull << (index & 0x3fu);
		if( (*mapEntryInUse & mapEntryInUseMask) == 0u )
		{
			break;
		}

		uint8* mapEntry = &hashMap->entries[ index * hashMap->entrySize ];
		if( hashMap->entryKeyEqualsFunc( mapEntry, entry ) )
		{
			return index;
		}
	}

	return IMUI_SIZE_MAX;
}

static bool ImUiHashMapGrow( ImUiHashMap* hashMap )
{
	uintsize nextCapacity = hashMap->entryCapacity;
	uint64* newEntriesInUse;
	uint8* newEntries;
	while( true )
	{
		nextCapacity <<= 1u;

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

			uint32 hashOffset = 0;
			for( ; hashOffset < 4u; ++hashOffset )
			{
				const uintsize newIndex = (hash + hashOffset) & (nextCapacity - 1u);

				uint64* newEntryInUse = &newEntriesInUse[ newIndex >> 6u ];
				const uint64 newEntryInUseMask = 1ull << (newIndex & 0x3fu);
				if( (*newEntriesInUse & newEntryInUseMask) != 0u )
				{
					continue;
				}

				*newEntryInUse |= newEntryInUseMask;

				uint8* newEntry = &newEntries[ newIndex * hashMap->entrySize ];
				memcpy( newEntry, mapEntry, hashMap->entrySize );
				break;
			}

			if( hashOffset >= 4u )
			{
				ImUiMemoryFree( hashMap->allocator, newEntriesInUse );
				ImUiMemoryFree( hashMap->allocator, newEntries );
				continue;
			}
		}

		break;
	}

	ImUiMemoryFree( hashMap->allocator, hashMap->entriesInUse );
	ImUiMemoryFree( hashMap->allocator, hashMap->entries );

	hashMap->entriesInUse	= newEntriesInUse;
	hashMap->entries		= newEntries;
	hashMap->entryCapacity	= nextCapacity;

	return true;
}

void* ImUiHashMapFind( ImUiHashMap* hashMap, const void* entry )
{
	const uintsize index = ImUiHashMapFindInternal( hashMap, entry );
	if( index >= hashMap->entryCapacity )
	{
		return NULL;
	}

	return &hashMap->entries[ index * hashMap->entrySize ];
}

void* ImUiHashMapInsert( ImUiHashMap* hashMap, const void* entry )
{
	return ImUiHashMapInsertNew( hashMap, entry, NULL );
}

void* ImUiHashMapInsertNew( ImUiHashMap* hashMap, const void* entry, bool* isNew )
{
	uint32 hashOffset = 0;
	while( true )
	{
		const ImUiHash hash = hashMap->entryHashFunc( entry );
		const uint32 index = (hash + hashOffset) & (hashMap->entryCapacity - 1u);

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
		if( hashOffset >= 4u &&
			!ImUiHashMapGrow( hashMap ) )
		{
			break;
		}
	}

	return NULL;
}

bool ImUiHashMapRemove( ImUiHashMap* hashMap, const void* entry )
{
	const uintsize index = ImUiHashMapFindInternal( hashMap, entry );
	if( index >= hashMap->entryCapacity )
	{
		return false;
	}

	uint64* mapEntryInUse = &hashMap->entriesInUse[ index >> 6u ];
	const uint64 mapEntryInUseMask = 1ull << (index & 0x3fu);

	*mapEntryInUse &= ~mapEntryInUseMask;
	hashMap->entryCount--;

	return true;
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
	return ImUiHashString( *string, 0u );
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
