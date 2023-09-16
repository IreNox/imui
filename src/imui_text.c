#include "imui_text.h"

#include "imui_font.h"
#include "imui_memory.h"

#include <string.h>

static ImUiHash ImUiTextLayoutCacheHash( const void* entry )
{
	const ImUiTextLayout* layout = *(const ImUiTextLayout**)entry;

	const ImUiHash fontHash = ImUiHashCreate( &layout->font, sizeof( &layout->font ), 0u );
	return ImUiHashString( layout->text, fontHash );
}

static bool ImUiTextLayoutCacheIsKeyEquals( const void* lhs, const void* rhs )
{
	const ImUiTextLayout* lhsLayout = *(const ImUiTextLayout**)lhs;
	const ImUiTextLayout* rhsLayout = *(const ImUiTextLayout**)rhs;

	if( lhsLayout->font != rhsLayout->font )
	{
		return false;
	}

	if( lhsLayout->text.length != rhsLayout->text.length )
	{
		return false;
	}
	else if( lhsLayout->text.length == 0u )
	{
		return true;
	}
	else if( lhsLayout->text.data[ 0u ] != rhsLayout->text.data[ 0u ] )
	{
		return false;
	}

	return memcmp( lhsLayout->text.data, rhsLayout->text.data, lhsLayout->text.length ) == 0;
}

bool ImUiTextLayoutCacheConstruct( ImUiTextLayoutCache* cache, ImUiAllocator* allocator )
{
	cache->allocator = allocator;

	if( !ImUiChunkedPoolConstruct( &cache->layoutPool, allocator, sizeof( ImUiTextLayout ), IMUI_DEFAULT_TEXT_LAYOUT_POOL_CHUNK_SIZE ) ||
		!ImUiHashMapConstructSize( &cache->layoutMap, allocator, sizeof( ImUiTextLayout* ), ImUiTextLayoutCacheHash, ImUiTextLayoutCacheIsKeyEquals, 64u ) )
	{
		ImUiTextLayoutCacheDestruct( cache );
		return false;
	}

	return true;
}

void ImUiTextLayoutCacheDestruct( ImUiTextLayoutCache* cache )
{
	for( uintsize i = ImUiHashMapFindFirstIndex( &cache->layoutMap ); i != IMUI_SIZE_MAX; i = ImUiHashMapFindNextIndex( &cache->layoutMap, i ) )
	{
		ImUiTextLayout* layout = *(ImUiTextLayout**)ImUiHashMapGetEntry( &cache->layoutMap, i );

		ImUiMemoryFree( cache->allocator, layout->glyphs );
	}

	ImUiHashMapDestruct( &cache->layoutMap );
	ImUiChunkedPoolDestruct( &cache->layoutPool );
}

void ImUiTextLayoutCacheFreeUnused( ImUiTextLayoutCache* cachce )
{

}

ImUiTextLayout* ImUiTextLayoutCreate( ImUiContext* imui, ImUiFont* font, ImUiStringView text )
{
	ImUiTextLayoutParameters parameters;
	parameters.font	= font;
	parameters.text	= text;

	return ImUiTextLayoutCacheCreateLayout( &imui->layoutCache, &parameters );
}

ImUiTextLayout* ImUiTextLayoutCacheCreateLayout( ImUiTextLayoutCache* cache, const ImUiTextLayoutParameters* parameters )
{
	bool isNew = false;
	ImUiTextLayout** mapLayout = (ImUiTextLayout**)ImUiHashMapInsertNew( &cache->layoutMap, &parameters, &isNew );
	if( !mapLayout )
	{
		return NULL;
	}

	if( !isNew )
	{
		return *mapLayout;
	}

	uint32 glyphCount = 0u;
	for( uintsize i = 0; i < parameters->text.length; )
	{
		const char c = parameters->text.data[ i ];
		const uint32 codepointByteCount = IMUI_COUNT_LEADING_ZEROS( ~((uint32)c << 24u) );

		i += IMUI_MAX( codepointByteCount, 1 );
		glyphCount++;
	}

	const uintsize extraMemorySize = parameters->text.length + 1u + (sizeof( ImUiTextGlyph ) * glyphCount);
	ImUiTextGlyph* glyphs = (ImUiTextGlyph*)ImUiMemoryAlloc( cache->allocator, extraMemorySize );
	if( !glyphs )
	{
		ImUiHashMapRemove( &cache->layoutMap, mapLayout );
		return NULL;
	}

	ImUiTextLayout* layout = (ImUiTextLayout*)ImUiChunkedPoolAllocate( &cache->layoutPool );
	if( !layout )
	{
		ImUiHashMapRemove( &cache->layoutMap, mapLayout );
		ImUiMemoryFree( cache->allocator, glyphs );
		return NULL;
	}

	char* textData = (char*)&glyphs[ glyphCount ];
	strcpy( textData, parameters->text.data );

	uintsize glyphIndex = 0u;
	float x = 0.0f;
	float height = 0.0f;
	for( uintsize i = 0; i < parameters->text.length; ++i )
	{
		ImUiTextGlyph* glyph = &glyphs[ glyphIndex ];

		const char c = parameters->text.data[ i ];
		const uint32 codepointByteCount = IMUI_COUNT_LEADING_ZEROS( ~((uint32)c << 24u) );
		const uint32 codepointMask = (1u << (8 - codepointByteCount)) - 1u;

		uint32 codepoint = c & codepointMask;
		for( int remainingBytes = (int)codepointByteCount - 1; remainingBytes > 0; --remainingBytes )
		{
			i++;
			if( i >= parameters->text.length )
			{
				break;
			}
			const char c2 = parameters->text.data[ i ];

			codepoint <<= 6u;
			codepoint += (c2 &0x3fu);
		}

		uint32* mapCodepointKey = &codepoint;
		ImUiFontCodepoint** mapCodepoint = (ImUiFontCodepoint**)ImUiHashMapFind( &parameters->font->codepointMap, &mapCodepointKey );
		if( !mapCodepoint )
		{
			codepoint = 0xfffd; // invalid codepoint
			mapCodepoint = (ImUiFontCodepoint**)ImUiHashMapFind( &parameters->font->codepointMap, &mapCodepointKey );
			if( !mapCodepoint )
			{

			}
		}

		ImUiFontCodepoint* codepointInfo;
		if( mapCodepoint )
		{
			codepointInfo = *mapCodepoint;
		}
		else
		{
			codepointInfo = NULL;
		}

		glyph->codepoint	= codepoint;
		glyph->position		= ImUiPositionCreate( x, codepointInfo->ascentOffset );
		glyph->size			= ImUiSizeCreate( (float)codepointInfo->width, (float)codepointInfo->height );
		glyph->uv			= codepointInfo->uv;

		glyphIndex++;
		x += codepointInfo->advance;
		height = IMUI_MAX( height, glyph->size.height );
	}

	layout->font		= parameters->font;
	layout->text.data	= textData;
	layout->text.length	= parameters->text.length;
	layout->glyphs		= glyphs;
	layout->glyphCount	= glyphCount;
	layout->size		= ImUiSizeCreate( x, height );

	*mapLayout = layout;
	return layout;
}

ImUiSize ImUiTextLayoutCacheMesureTextSize( ImUiTextLayoutCache* cache, const ImUiTextLayoutParameters* parameters )
{
	ImUiTextLayout* layout = ImUiTextLayoutCacheCreateLayout( cache, parameters );
	if( !layout )
	{
		return ImUiSizeCreateZero();
	}

	return ImUiTextLayoutGetSize( layout );
}

ImUiSize ImUiTextLayoutGetSize( const ImUiTextLayout* layout )
{
	return layout->size;
}
