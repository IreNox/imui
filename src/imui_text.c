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

	if( !ImUiHashMapConstructSize( &cache->layoutMap, allocator, sizeof( ImUiTextLayout* ), ImUiTextLayoutCacheHash, ImUiTextLayoutCacheIsKeyEquals, 64u ) )
	{
		ImUiTextLayoutCacheDestruct( cache );
		return false;
	}

	return true;
}

void ImUiTextLayoutCacheDestruct( ImUiTextLayoutCache* cache )
{
	ImUiTextLayout* layout = cache->firstLayout;
	ImUiTextLayout* nextLayout = NULL;
	while( layout )
	{
		nextLayout = layout->nextLayout;
		ImUiMemoryFree( cache->allocator, layout );
		layout = nextLayout;
	}

	layout = cache->firstUnusedLayout;
	while( layout )
	{
		nextLayout = layout->nextLayout;
		ImUiMemoryFree( cache->allocator, layout );
		layout = nextLayout;
	}

	ImUiHashMapDestruct( &cache->layoutMap );
}

void ImUiTextLayoutCacheEndFrame( ImUiTextLayoutCache* cache )
{
	ImUiTextLayout* unusedLayout = cache->firstUnusedLayout;
	ImUiTextLayout* nextUnusedLayout = NULL;
	while( unusedLayout )
	{
		nextUnusedLayout = unusedLayout->nextLayout;

		const bool removed = ImUiHashMapRemove( &cache->layoutMap, &unusedLayout );
		IMUI_ASSERT( removed );
		ImUiMemoryFree( cache->allocator, unusedLayout );
		unusedLayout = nextUnusedLayout;
	}
	cache->firstUnusedLayout	= cache->firstLayout;
	cache->firstLayout			= NULL;

	cache->frameIndex++;
}

ImUiTextLayout* ImUiTextLayoutCreate( ImUiContext* imui, ImUiFont* font, ImUiStringView text )
{
	ImUiTextLayoutParameters parameters;
	parameters.font	= font;
	parameters.text	= text;

	return ImUiTextLayoutCacheCreateLayout( &imui->layoutCache, &parameters );
}

ImUiTextLayout* ImUiTextLayoutCreateWidget( ImUiWidget* widget, ImUiFont* font, ImUiStringView text )
{
	return ImUiTextLayoutCreate( widget->window->imui, font, text );
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
		ImUiTextLayout* layout = *mapLayout;
		if( layout->frameIndex != cache->frameIndex )
		{
			if( layout->prevLayout )
			{
				layout->prevLayout->nextLayout = layout->nextLayout;
			}
			else
			{
				IMUI_ASSERT( cache->firstUnusedLayout == layout );
				cache->firstUnusedLayout = layout->nextLayout;
			}

			if( layout->nextLayout )
			{
				layout->nextLayout->prevLayout = layout->prevLayout;
			}

			layout->nextLayout = cache->firstLayout;
			layout->prevLayout = NULL;

			if( cache->firstLayout )
			{
				cache->firstLayout->prevLayout = layout;
			}
			cache->firstLayout = layout;

			layout->frameIndex = cache->frameIndex;
		}

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

	const uintsize memorySize = sizeof( ImUiTextLayout ) + parameters->text.length + 1u + (sizeof( ImUiTextGlyph ) * glyphCount);
	ImUiTextLayout* layout = (ImUiTextLayout*)ImUiMemoryAlloc( cache->allocator, memorySize );
	if( !layout )
	{
		ImUiHashMapRemove( &cache->layoutMap, mapLayout );
		return NULL;
	}

	ImUiTextGlyph* glyphs = (ImUiTextGlyph*)&layout[ 1u ];
	char* textData = (char*)&glyphs[ glyphCount ];
	memcpy( textData, parameters->text.data, parameters->text.length + 1u );

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
		glyph->pos			= ImUiPosCreate( x, codepointInfo->ascentOffset );
		glyph->size			= ImUiSizeCreate( (float)codepointInfo->width, (float)codepointInfo->height );
		glyph->uv			= codepointInfo->uv;

		glyphIndex++;
		x += codepointInfo->advance;
		height = IMUI_MAX( height, glyph->pos.y + glyph->size.height );
	}

	layout->font		= parameters->font;
	layout->text.data	= textData;
	layout->text.length	= parameters->text.length;
	layout->glyphs		= glyphs;
	layout->glyphCount	= glyphCount;
	layout->size		= ImUiSizeCreate( x, height );
	layout->frameIndex	= cache->frameIndex;

	layout->prevLayout	= NULL;
	layout->nextLayout	= cache->firstLayout;

	if( cache->firstLayout )
	{
		cache->firstLayout->prevLayout = layout;
	}
	cache->firstLayout = layout;

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
	if( !layout )
	{
		return ImUiSizeCreateZero();
	}

	return layout->size;
}
