#include "imui_string_pool.h"

#include "imui_internal.h"
#include "imui_memory.h"

#include <string.h>

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
