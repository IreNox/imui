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

void ImUiStringPoolConstruct( ImUiStringPool* stringPool, ImUiAllocator* allocator )
{
	stringPool->allocator	= allocator;
	stringPool->firstChunk	= NULL;
}

void ImUiStringPoolDestruct( ImUiStringPool* stringPool )
{
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

ImUiStringView ImUiStringPoolAdd( ImUiStringPool* stringPool, ImUiStringView string )
{
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
			const ImUiStringView result = { NULL, 0u };
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

	const ImUiStringView result = { target, string.length };
	return result;
}
