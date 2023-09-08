#include "imui_helpers.h"

#include "imui_internal.h"
#include "imui_memory.h"

#include <string.h>

static const void* ImUiHashTableGetKey( ImUiHashTable* hashTable, const void* element )
{
	const uint8* data = (const uint8*)element;
	return data + hashTable->keyOffset;
}

bool ImUiHashTableCreateSize( ImUiAllocator* allocator, ImUiHashTable* hashTable, uintsize keyOffset, uintsize keySize, uintsize initialSize )
{
	// round up to next power of 2
	initialSize--;
	initialSize |= initialSize >> 1;
	initialSize |= initialSize >> 2;
	initialSize |= initialSize >> 4;
	initialSize |= initialSize >> 8;
	initialSize |= initialSize >> 16;
	initialSize++;

	hashTable->allocator	= allocator;
	hashTable->indices		= IMUI_MEMORY_ARRAY_NEW_ZERO( allocator, uint32, initialSize );
	hashTable->keys			= (uint8*)ImUiMemoryAlloc( allocator, initialSize * keySize );
	hashTable->keyCount		= 0u;
	hashTable->keyCapacity	= initialSize;
	hashTable->keyOffset	= keyOffset;
	hashTable->keySize		= keySize;

	if( !hashTable->indices || !hashTable->keys )
	{
		ImUiHashTableDestroy( hashTable );
		return false;
	}

	return true;
}

bool ImUiHashTableCreateStatic( ImUiAllocator* allocator, ImUiHashTable* hashTable, const void* data, uintsize elementSize, uintsize elementCount, uintsize keyOffset, uintsize keySize )
{
	if( !ImUiHashTableCreateSize( allocator, hashTable, keyOffset, keySize, elementCount * 2u ) )
	{
		return false;
	}

	const uint8* element = (const uint8*)data;
	for( uint32 i = 0; i < elementCount; ++i )
	{
		if( !ImUiHashTableInsert( hashTable, element, i ) )
		{
			ImUiHashTableDestroy( hashTable );
			return false;
		}

		element += elementSize;
	}

	return true;
}

void ImUiHashTableDestroy( ImUiHashTable* hashTable )
{
	ImUiMemoryFree( hashTable->allocator, hashTable->indices );
	ImUiMemoryFree( hashTable->allocator, hashTable->keys );

	hashTable->indices		= NULL;
	hashTable->keys			= NULL;
	hashTable->keyCapacity	= 0u;
	hashTable->allocator	= NULL;
}

static uint32* ImUiHashTableFindInternal( ImUiHashTable* hashTable, const void* element )
{
	uint32 hashSeed = 0u;
	const void* key = ImUiHashTableGetKey( hashTable, element );
	while( true )
	{
		const ImUiHash hash = ImUiHashCreate( key, hashTable->keySize, hashSeed );
		const uintsize index = hash & (hashTable->keyCapacity - 1u);

		uint32* mapIndex = &hashTable->indices[ index ];
		if( (*mapIndex & 0x80000000) == 0u )
		{
			break;
		}

		const uint8* mapKey = &hashTable->keys[ index * hashTable->keySize ];
		if( memcmp( mapKey, key, hashTable->keySize ) == 0 )
		{
			return mapIndex;
		}

		hashSeed++;
	}

	return NULL;
}

static bool ImUiHashTableGrow( ImUiHashTable* hashTable )
{
	const uintsize nextCapacity = hashTable->keyCapacity << 1u;

	uint32* newIndices = IMUI_MEMORY_ARRAY_NEW_ZERO( hashTable->allocator, uint32, nextCapacity );
	uint8* newKeys = (uint8*)ImUiMemoryAlloc( hashTable->allocator, nextCapacity * hashTable->keySize );
	if( !newIndices || !newKeys )
	{
		return false;
	}

	const uint32* mapIndex = hashTable->indices;
	for( uintsize i = 0; i < hashTable->keyCapacity; ++i, ++mapIndex )
	{
		if( (*mapIndex & 0x80000000) == 0u )
		{
			continue;
		}

		uint32 hashSeed = 0;
		const uint8* mapKey = &hashTable->keys[ i * hashTable->keySize ];
		for( ; hashSeed < 4u; ++hashSeed )
		{
			const ImUiHash hash = ImUiHashCreate( mapKey, hashTable->keySize, hashSeed );
			const uintsize index = hash & (nextCapacity - 1u);

			uint32* newIndex = &newIndices[ index ];
			uint8* newKey = &newKeys[ index * hashTable->keySize ];

			if( *mapIndex & 0x80000000 )
			{
				continue;
			}

			*newIndex = *mapIndex;
			memcpy( newKey, mapKey, hashTable->keySize );
			break;
		}

		if( hashSeed >= 4u )
		{
			ImUiMemoryFree( hashTable->allocator, newIndices );
			ImUiMemoryFree( hashTable->allocator, newKeys );
			return false;
		}
	}

	ImUiMemoryFree( hashTable->allocator, hashTable->indices );
	ImUiMemoryFree( hashTable->allocator, hashTable->keys );

	hashTable->indices		= newIndices;
	hashTable->keys			= newKeys;
	hashTable->keyCapacity	= nextCapacity;

	return true;
}

const uint32* ImUiHashTableFind( ImUiHashTable* hashTable, const void* element )
{
	return ImUiHashTableFindInternal( hashTable, element );
}

bool ImUiHashTableInsert( ImUiHashTable* hashTable, const void* element, uint32 index )
{
	IMUI_ASSERT( (index & 0x8000000) == 0u );

	uint32 hashSeed = 0u;
	const void* key = ImUiHashTableGetKey( hashTable, element );
	while( true )
	{
		const ImUiHash hash = ImUiHashCreate( key, hashTable->keySize, hashSeed );
		const uint32 index = hash & (hashTable->keyCapacity - 1u);

		uint32* mapIndex = &hashTable->indices[ index ];
		uint8* mapKey = &hashTable->keys[ index * hashTable->keySize ];

		if( (*mapIndex & 0x80000000) == 0u )
		{
			*mapIndex = index | 0x80000000;
			memcpy( mapKey, key, hashTable->keySize );

			hashTable->keyCount++;
			return true;
		}

		if( memcmp( mapKey, key, hashTable->keySize ) == 0 )
		{
			return true;
		}

		hashSeed++;
		if( hashSeed > 4u )
		{
			if( !ImUiHashTableGrow( hashTable ) )
			{
				return false;
			}
		}
	}

	return false;
}

bool ImUiHashTableRemove( ImUiHashTable* hashTable, const void* element )
{
	uint32* mapIndex = ImUiHashTableFindInternal( hashTable, element );
	if( !mapIndex )
	{
		return false;
	}

	*mapIndex = 0u;
	hashTable->keyCount--;
	return true;
}

struct ImUiStringPoolChunk
{
	ImUiStringPoolChunk*	nextChunk;

	uintsize				usedSize;
	uintsize				remainingSize;
	char					data[ 1u ];
};

struct ImUiStringPoolKey
{
	ImUiHash				stringHash;
	ImUiStringView			string;
};

void ImUiStringPoolConstruct( ImUiStringPool* stringPool, ImUiAllocator* allocator )
{
	stringPool->allocator	= allocator;
	stringPool->firstChunk	= NULL;
}

void ImUiStringPoolDestruct( ImUiStringPool* stringPool )
{
	ImUiMemoryFree( stringPool->allocator, stringPool->keys );
	stringPool->keys		= NULL;
	stringPool->keyCount	= 0u;
	stringPool->keyCapacity	= 0u;

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
	IMUI_ASSERT( string.length <= ((size_t)-1) >> 1u ); // use upper bit as used flag

	const ImUiHash stringHash = ImUiHashString( string, 0u );
	ImUiStringView result = ImUiStringPoolFindByHash( stringPool, stringHash );
	if( result.data )
	{
		return result;
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
			return result;
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

	result.data		= target;
	result.length	= string.length;

	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY( stringPool->allocator, stringPool->keys, stringPool->keyCapacity, stringPool->keyCount + 1u ) )
	{
		return result;
	}

	ImUiStringPoolKey* key = &stringPool->keys[ stringPool->keyCount ];
	stringPool->keyCount++;

	key->stringHash	= stringHash;
	key->string		= result;

	return result;
}

ImUiStringView ImUiStringPoolFindByHash( ImUiStringPool* stringPool, ImUiHash stringHash )
{
	ImUiStringView result = { NULL, 0u };
	for( size_t i = 0u; i < stringPool->keyCount; ++i )
	{
		ImUiStringPoolKey* key = &stringPool->keys[ i ];
		if( key->stringHash != stringHash )
		{
			continue;
		}

		result.data		= key->string.data;
		result.length	= key->string.length; // TODO: &~(((size_t)-1) >> 1u);
		return result;
	}

	return result;
}
